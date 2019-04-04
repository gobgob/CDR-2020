#ifndef ACTUATOR_MGR_h
#define ACTUATOR_MGR_h

#include <A4988.h>
#include <ToF_sensor.h>
#include <Dynamixel.h>
#include <DynamixelInterface.h>
#include <DynamixelMotor.h>
#include <Printable.h>
#include <vector>
#include "SerialAX12.h"
#include "Singleton.h"
#include "Serializer.h"
#include "Config.h"

#define ACTUATOR_MGR_INTERRUPT_PERIOD   (1000)  // µs
#define ACTUATOR_MGR_POLL_PERIOD        (5000)  // µs
#define ACTUATOR_MGR_MOVE_TIMEOUT       (8000)  // ms
#define ACTUATOR_MGR_Y_TOLERANCE        (1.5)   // mm
#define ACTUATOR_MGR_Z_TOLERANCE        (1.0)   // mm
#define ACTUATOR_MGR_THETA_TOLERANCE    (5)     // deg
#define ACTUATOR_MGR_Y_MIN              (-50)   // mm (todo: set value)
#define ACTUATOR_MGR_Y_MAX              (50)    // mm (todo: set value)
#define ACTUATOR_MGR_Z_MIN              (0)     // mm
#define ACTUATOR_MGR_Z_MAX              (300)   // mm (todo: set value)
#define ACTUATOR_MGR_THETA_MIN          (60)    // deg (todo: set value)
#define ACTUATOR_MGR_THETA_MAX          (240)   // deg (todo: set value)
#define ACTUATOR_MGR_Y_ORIGIN           (150)   // deg (Angle de l'AX12 de l'axe Y pour une fourche centrée)
#define ACTUATOR_MGR_Y_CONVERTER        (2.0)   // deg/mm (Conversion Y <-> Angle d'AX12) (=360/(PI * d)) (todo: set value)
#define ACTUATOR_MGR_THETA_ORIGIN       (150)   // deg (Angle de l'AX12 theta pour une fourche horizontale)
#define ACTUATOR_MGR_SENSE_MIN_THETA    (130)   // deg (Angle minimal de l'AX12 theta pour utiliser les capteurs de fourche)
#define ACTUATOR_MGR_SENSE_MAX_THETA    (170)   // deg (Angle maximal de l'AX12 theta pour utiliser les capteurs de fourche)


typedef int32_t ActuatorErrorCode;
enum ActuatorError
{
    ACT_OK                  = 0x0000,
    ACT_STEPPER_BLOCKED     = 0x0001,
    ACT_AX12_Y_BLOCKED      = 0x0002,
    ACT_AX12_THETA_BLOCKED  = 0x0004,
    ACT_AX12_ERROR          = 0x0008,
    ACT_STOP_REQUESTED      = 0x0010,
    ACT_UNREACHABLE         = 0x0020,
    ACT_SENSOR_ERROR        = 0x0040,
    ACT_NO_DETECTION        = 0x0080,
    ACT_TIMED_OUT           = 0x0100,
    ACT_ALREADY_MOVING      = 0x0200,
};


class ActuatorPosition : public Printable
{
public:
    ActuatorPosition()
    {
        y = 0;
        z = 0;
        theta = 0;
    }

    bool isEqualTo(const ActuatorPosition & p) const
    {
        return
            p.y >= y - ACTUATOR_MGR_Y_TOLERANCE &&
            p.y <= y + ACTUATOR_MGR_Y_TOLERANCE &&
            p.z >= z - ACTUATOR_MGR_Z_TOLERANCE &&
            p.z <= z + ACTUATOR_MGR_Z_TOLERANCE &&
            p.theta >= theta - ACTUATOR_MGR_THETA_TOLERANCE &&
            p.theta <= theta + ACTUATOR_MGR_THETA_TOLERANCE;
    }

    bool isWithinRange() const
    {
        return
            y >= ACTUATOR_MGR_Y_MIN && y <= ACTUATOR_MGR_Y_MAX &&
            z >= ACTUATOR_MGR_Z_MIN && z <= ACTUATOR_MGR_Z_MAX &&
            theta >= ACTUATOR_MGR_THETA_MIN && theta <= ACTUATOR_MGR_THETA_MAX;
    }

    /*
        y (mm) déplacement latéral
        y=0: fourche centrée
        y>0: décalage à gauche
        y<0: décalage à droite
    */
    float y;

    /*
        z (mm) déplacement vertical
        z=0: fourche au niveau du sol
        z>0: translation vers le haut
    */
    float z;

    /*
        theta (deg) angle de la fourche
        theta=0: fourche horizontale
        theta>0: pointe vers le haut (conservation du palet)
        theta<0: pointe vers le bas (éjection du palet)
    */
    float theta;

    size_t printTo(Print& p) const
    {
        return p.printf("%g_%g_%g", y, z, theta);
    }
};


class ActuatorMgr : public Singleton<ActuatorMgr>
{
public:
    ActuatorMgr() :
        m_y_motor(SerialAX12, ID_AX12_ACT_Y),
        m_theta_motor(SerialAX12, ID_AX12_ACT_THETA)
    {
        m_error_code = ACT_OK;
        m_status = STATUS_IDLE;
        m_left_sensor_value = (SensorValue)SENSOR_DEAD;
        m_right_sensor_value = (SensorValue)SENSOR_DEAD;
        m_composed_move_step = 0;
        m_move_start_time = 0;
        m_scan_enabled = false;
        attachInterrupt(PIN_STEPPER_ENDSTOP, endstopInterrupt, CHANGE);
    }

    int init()
    {
        if (m_y_motor.init() != DYN_STATUS_OK ||
                m_theta_motor.init() != DYN_STATUS_OK) {
            return EXIT_FAILURE;
        }
        m_y_motor.enableTorque();
        m_y_motor.jointMode();
        m_theta_motor.enableTorque();
        m_theta_motor.jointMode();
        return EXIT_SUCCESS;
    }

    void mainLoopControl()
    {
        readSensorsAndMotors();
        switch (m_status)
        {
        case STATUS_MOVING:
            simpleMoveHandler();
            break;
        case STATUS_GOING_HOME:
            goHomeHandler();
            break;
        case STATUS_SCANNING:
            scanningHandler();
            break;
        default:
            break;
        }
        if (m_status != STATUS_IDLE)
        {
            if (millis() - m_move_start_time > ACTUATOR_MGR_MOVE_TIMEOUT)
            {
                m_error_code |= ACT_TIMED_OUT;
                stopMove(false);
            }
        }
    }

    void interruptControl()
    {
        // todo: control stepper
    }

    bool commandCompleted() const
    {
        return m_status == STATUS_IDLE;
    }

    ActuatorErrorCode getErrorCode() const
    {
        return m_error_code;
    }

    void getSensorsValues(SensorValue &left, SensorValue &right) const
    {
        if (canUseSensors())
        {
            left = m_left_sensor_value;
            right = m_right_sensor_value;
        }
        else
        {
            left = (SensorValue)SENSOR_DEAD;
            right = (SensorValue)SENSOR_DEAD;
        }
    }

    void appendSensorsValuesToVect(std::vector<uint8_t> & output) const
    {
        if (canUseSensors())
        {
            Serializer::writeInt(m_left_sensor_value, output);
            Serializer::writeInt(m_right_sensor_value, output);
        }
        else
        {
            Serializer::writeInt((SensorValue)SENSOR_DEAD, output);
            Serializer::writeInt((SensorValue)SENSOR_DEAD, output);
        }
    }

    /* Mouvement control */
    void stop() { stopMove(true); }
    int goToHome() { return initMove(STATUS_GOING_HOME, m_current_position); }
    int goToPosition(const ActuatorPosition &p) { return initMove(STATUS_MOVING, p); }
    int scanPuck() { return initMove(STATUS_SCANNING, m_current_position); }

private:
    enum ActuatorStatus
    {
        STATUS_IDLE,
        STATUS_MOVING,
        STATUS_GOING_HOME,
        STATUS_SCANNING,
    };

    static void endstopInterrupt()
    {

    }

    int initMove(ActuatorStatus moveId, const ActuatorPosition &p)
    {
        if (m_status != STATUS_IDLE)
        {
            return EXIT_FAILURE;
        }
        m_error_code = ACT_OK;
        if (!p.isWithinRange())
        {
            m_error_code |= ACT_UNREACHABLE;
        }
        else
        {
            m_status = moveId;
            m_aim_position = p;
            sendAimPosition();
            m_move_start_time = millis();
        }
        return EXIT_SUCCESS;
    }

    void stopMove(bool putFlag)
    {
        if (putFlag) {
            m_error_code |= ACT_STOP_REQUESTED;        
        }
        m_status = STATUS_IDLE;
        m_composed_move_step = 0;
        m_scan_enabled = false;
        // todo: reset scan structure
        m_aim_position = m_current_position;
        sendAimPosition();
        // todo: stop stepper motor
    }

    void finishMove()
    {
        m_status = STATUS_IDLE;
        m_composed_move_step = 0;
        m_scan_enabled = false;
        // todo: reset scan structure
    }

    void simpleMoveHandler()
    {
        if (aimPositionReached())
        {
            finishMove();
        }
    }

    void goHomeHandler()
    {
        // todo
    }

    void scanningHandler()
    {

    }

    bool aimPositionReached() const
    {
        return m_current_position.isEqualTo(m_aim_position);
    }

    bool canUseSensors() const
    {
        return m_current_position.theta > ACTUATOR_MGR_SENSE_MIN_THETA &&
            m_current_position.theta < ACTUATOR_MGR_SENSE_MAX_THETA;
    }

    void sendAimPosition()
    {
        DynamixelStatus dynamixelStatus;
        dynamixelStatus = m_y_motor.goalPositionDegree(
            m_aim_position.y * ACTUATOR_MGR_Y_CONVERTER + ACTUATOR_MGR_Y_ORIGIN);
        if (dynamixelStatus != DYN_STATUS_OK) {
            m_error_code |= ACT_AX12_ERROR;
        }
        dynamixelStatus = m_theta_motor.goalPositionDegree(
            m_aim_position.theta + ACTUATOR_MGR_THETA_ORIGIN);
        if (dynamixelStatus != DYN_STATUS_OK) {
            m_error_code |= ACT_AX12_ERROR;
        }

        // todo: set aim position for stepper
    }

    void readSensorsAndMotors()
    {
        static uint32_t last_poll_time = 0;
        static uint8_t step = 0;

        uint32_t now = micros();
        if (now - last_poll_time > ACTUATOR_MGR_POLL_PERIOD)
        {
            last_poll_time = now;
            if (step == 0)
            {
                uint16_t angle;
                DynamixelStatus dynamixelStatus = m_y_motor.currentPositionDegree(angle);
                if (angle <= 300) {
                    m_current_position.y = ((float)angle - ACTUATOR_MGR_Y_ORIGIN) / ACTUATOR_MGR_Y_CONVERTER;
                }
                if (dynamixelStatus & (DYN_STATUS_OVERLOAD_ERROR | DYN_STATUS_OVERHEATING_ERROR)) {
                    m_error_code |= ACT_AX12_Y_BLOCKED;
                }
                else if (dynamixelStatus != DYN_STATUS_OK) {
                    m_error_code |= ACT_AX12_ERROR;
                }
                step++;
            }
            else if (step == 1)
            {
                uint16_t angle;
                DynamixelStatus dynamixelStatus = m_theta_motor.currentPositionDegree(angle);
                if (angle <= 300) {
                    m_current_position.theta = (float)angle - ACTUATOR_MGR_THETA_ORIGIN;
                }
                if (dynamixelStatus & (DYN_STATUS_OVERLOAD_ERROR | DYN_STATUS_OVERHEATING_ERROR)) {
                    m_error_code |= ACT_AX12_THETA_BLOCKED;
                }
                else if (dynamixelStatus != DYN_STATUS_OK) {
                    m_error_code |= ACT_AX12_ERROR;
                }
                step++;
            }
            else if (step == 2)
            {
                SensorValue val = m_left_sensor.getMeasure();
                if (val != SENSOR_NOT_UPDATED) {
                    m_left_sensor_value = val;
                    if (m_scan_enabled) {
                        // todo: store value in scan structure
                    }
                }
                step++;
            }
            else if (step == 3)
            {
                SensorValue val = m_right_sensor.getMeasure();
                if (val != SENSOR_NOT_UPDATED) {
                    m_right_sensor_value = val;
                    if (m_scan_enabled) {
                        // todo: store value in scan structure
                    }
                }
                step = 0;
            }

            if (step > 1 && !canUseSensors()) {
                step = 0;
            }
        }
    }

    ActuatorPosition m_current_position;
    ActuatorPosition m_aim_position;
    ActuatorErrorCode m_error_code;
    ActuatorStatus m_status;
    ToF_shortRange m_left_sensor;
    ToF_shortRange m_right_sensor;
    SensorValue m_left_sensor_value;
    SensorValue m_right_sensor_value;
    DynamixelMotor m_y_motor;
    DynamixelMotor m_theta_motor;
    // todo: add stepper motor
    uint32_t m_composed_move_step;
    uint32_t m_move_start_time; // ms
    bool m_scan_enabled;
    // todo: add structure to store scan data
};

#endif
