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
#include "CommunicationServer.h"
#include "PuckScanner.h"

#define ACT_MGR_INTERRUPT_PERIOD    (100)       // µs
#define ACT_MGR_POLL_PERIOD         (5000)      // µs
#define ACT_MGR_MOVE_TIMEOUT        (8000)      // ms
#define ACT_MGR_Y_TOLERANCE         (1.5)       // mm
#define ACT_MGR_Z_TOLERANCE         (1.0)       // mm
#define ACT_MGR_THETA_TOLERANCE     (5)         // deg
#define ACT_MGR_Y_MIN               (-23.795)   // mm (47.59 / 2)
#define ACT_MGR_Y_MAX               (23.795)    // mm
#define ACT_MGR_Z_MIN               (0)         // mm
#define ACT_MGR_Z_MAX               (208)       // mm (todo: adjust with the base)
#define ACT_MGR_THETA_MIN           (-90)       // deg
#define ACT_MGR_THETA_MAX           (40)        // deg
#define ACT_MGR_Y_ORIGIN            (150)       // deg (Angle de l'AX12 de l'axe Y pour une fourche centrée)
#define ACT_MGR_Y_CONVERTER         (2.8233)    // deg/mm (Conversion Y <-> Angle d'AX12) (100 deg = 35.42mm)
#define ACT_MGR_THETA_ORIGIN        (150)       // deg (Angle de l'AX12 theta pour une fourche horizontale)
#define ACT_MGR_SENSE_MIN_THETA     (-20)       // deg (Angle minimal de l'AX12 theta pour utiliser les capteurs de fourche)
#define ACT_MGR_SENSE_MAX_THETA     (20)        // deg (Angle maximal de l'AX12 theta pour utiliser les capteurs de fourche)
#define ACT_MGR_STEPPER_SPEED       (200)       // rmp
#define ACT_MGR_MICROSTEP           (16)
#define ACT_MGR_STEP_PER_TURN       (200)       // step/turn
#define ACT_MGR_Z_PER_TURN          (8)         // mm/turn
#define ACT_MGR_SCAN_RESOLUTION     (101)       // resolution spaciale selon l'axe Y
#define ACT_MGR_SCAN_AVG_SIZE       (5)         // taille de la moyenne mobile du scan
#define ACT_MGR_HALF_FORK_DIST      (33.0)      // mm (Demi distance entre les deux fourches)
#define ACT_MGR_SENSOR_MIN          (15)        // mm
#define ACT_MGR_SENSOR_MAX          (200)       // mm


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
            p.y >= y - ACT_MGR_Y_TOLERANCE &&
            p.y <= y + ACT_MGR_Y_TOLERANCE &&
            p.z >= z - ACT_MGR_Z_TOLERANCE &&
            p.z <= z + ACT_MGR_Z_TOLERANCE &&
            p.theta >= theta - ACT_MGR_THETA_TOLERANCE &&
            p.theta <= theta + ACT_MGR_THETA_TOLERANCE;
    }

    bool isWithinRange() const
    {
        return
            y >= ACT_MGR_Y_MIN && y <= ACT_MGR_Y_MAX &&
            z >= ACT_MGR_Z_MIN && z <= ACT_MGR_Z_MAX &&
            theta >= ACT_MGR_THETA_MIN && theta <= ACT_MGR_THETA_MAX;
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
        m_left_sensor(I2C_ADDR_TOF_FOURCHE_AVG, PIN_EN_TOF_FOURCHE_AVG, ACT_MGR_SENSOR_MIN, ACT_MGR_SENSOR_MAX, "FourcheG", &Serial),
        m_right_sensor(I2C_ADDR_TOF_FOURCHE_AVD, PIN_EN_TOF_FOURCHE_AVD, ACT_MGR_SENSOR_MIN, ACT_MGR_SENSOR_MAX, "FourcheD", &Serial),
        m_y_motor(SerialAX12, ID_AX12_ACT_Y),
        m_theta_motor(SerialAX12, ID_AX12_ACT_THETA),
        m_z_motor(ACT_MGR_STEP_PER_TURN, PIN_STEPPER_DIR, PIN_STEPPER_STEP,
            PIN_STEPPER_SLEEP, PIN_MICROSTEP_1, PIN_MICROSTEP_2, PIN_MICROSTEP_3)
    {
        m_error_code = ACT_OK;
        m_status = STATUS_IDLE;
        m_left_sensor_value = (SensorValue)SENSOR_DEAD;
        m_right_sensor_value = (SensorValue)SENSOR_DEAD;
        m_z_current_move_origin = 0;
        m_composed_move_step = 0;
        m_move_start_time = 0;
        m_scan_enabled = false;
        pinMode(PIN_STEPPER_ENDSTOP, INPUT);
    }

    int init()
    {
        int ret = EXIT_SUCCESS;

        if (m_left_sensor.powerON() != EXIT_SUCCESS) { ret = EXIT_FAILURE; }
        if (m_right_sensor.powerON() != EXIT_SUCCESS) { ret = EXIT_FAILURE; }

        if (m_y_motor.init() == DYN_STATUS_OK)
        {
            m_y_motor.enableTorque();
            m_y_motor.jointMode();
        }
        else { ret = EXIT_FAILURE; }
        if (m_theta_motor.init() == DYN_STATUS_OK)
        {
            m_theta_motor.enableTorque();
            m_theta_motor.jointMode();
        }
        else { ret = EXIT_FAILURE; }

        pinMode(PIN_STEPPER_RESET, OUTPUT);
        digitalWrite(PIN_STEPPER_RESET, HIGH);
        noInterrupts();
        m_z_motor.begin(ACT_MGR_STEPPER_SPEED, ACT_MGR_MICROSTEP);
        m_z_motor.setMicrostep(ACT_MGR_MICROSTEP);
        m_z_motor.enable();
        interrupts();

        return ret;
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
            if (millis() - m_move_start_time > ACT_MGR_MOVE_TIMEOUT)
            {
                m_error_code |= ACT_TIMED_OUT;
                stopMove(false);
            }
        }
    }

    void interruptControl()
    {
        static uint32_t last_action_time = 0;
        static uint32_t wait_time = 0;
        if (micros() - last_action_time > wait_time) {
            wait_time = m_z_motor.nextAction();
            last_action_time = micros();
        }
    }

    bool commandCompleted() const
    {
        return m_status == STATUS_IDLE;
    }

    ActuatorErrorCode getErrorCode() const
    {
        return m_error_code;
    }

    const ActuatorPosition &getPosition() const
    {
        return m_current_position;
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
            if (p.y < ACT_MGR_Y_MIN || p.y > ACT_MGR_Y_MAX) {
                Server.printf_err("y (%g) is out of range (%g;%g)", p.y, ACT_MGR_Y_MIN, ACT_MGR_Y_MAX);
            }
            if (p.z < ACT_MGR_Z_MIN || p.z > ACT_MGR_Z_MAX) {
                Server.printf_err("z (%g) is out of range (%g;%g)", p.z, ACT_MGR_Z_MIN, ACT_MGR_Z_MAX);
            }
            if (p.theta < ACT_MGR_THETA_MIN || p.theta > ACT_MGR_THETA_MAX) {
                Server.printf_err("theta (%g) is out of range (%g;%g)", p.theta, ACT_MGR_THETA_MIN, ACT_MGR_THETA_MAX);
            }
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
        m_raw_scan_data.clear();
        readZCurrentPosition();
        m_aim_position = m_current_position;
        sendAimPosition();
    }

    void finishMove()
    {
        m_status = STATUS_IDLE;
        m_composed_move_step = 0;
        m_scan_enabled = false;
        m_raw_scan_data.clear();
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
        switch (m_composed_move_step)
        {
        case 0:
            m_aim_position.y = 0;
            if (endstopPressed())
            {
                m_aim_position.z = m_current_position.z - 20;
                m_composed_move_step++;
            }
            else
            {
                m_aim_position.z = ACT_MGR_Z_MAX * 2;
                m_composed_move_step = 2;
            }
            sendAimPosition();
            break;
        case 1:
            if (!endstopPressed())
            {
                m_aim_position.z = ACT_MGR_Z_MAX * 2;
                sendAimPosition();
                m_composed_move_step++;
            }
            break;
        case 2:
            if (endstopPressed())
            {
                resetZOrigin();
                m_aim_position.z = m_current_position.z - 20; // Avoid to stress on the endstop, and it's more esthetic
                m_aim_position.theta = ACT_MGR_THETA_MIN;
                sendAimPosition();
                m_composed_move_step++;
            }
            else if (aimPositionReached())
            {
                m_error_code |= ACT_STEPPER_BLOCKED;
                finishMove();
            }
            break;
        case 3:
            if (aimPositionReached())
            {
                finishMove();
            }
            break;
        default:
            break;
        }
    }

    void scanningHandler()
    {
        switch (m_composed_move_step)
        {
        case 0:
            m_scan_enabled = true;
            m_aim_position.y = ACT_MGR_Y_MIN;
            sendAimPosition();
            m_composed_move_step++;
            break;
        case 1:
            if (aimPositionReached())
            {
                m_aim_position.y = ACT_MGR_Y_MAX;
                sendAimPosition();
                m_composed_move_step++;
            }
            break;
        case 2:
            if (aimPositionReached())
            {
                if (computeScanData(m_aim_position.y) != EXIT_SUCCESS)
                {
                    // On failure : set y to zero and raise error flag
                    m_aim_position.y = 0;
                    m_error_code |= ACT_NO_DETECTION;
                }
                m_scan_enabled = false;
                sendAimPosition();
                m_composed_move_step++;
            }
            break;
        case 3:
            if (aimPositionReached())
            {
                finishMove();
            }
            break;
        default:
            break;
        }
    }

    bool aimPositionReached() const
    {
        return m_current_position.isEqualTo(m_aim_position);
    }

    bool canUseSensors() const
    {
        return m_current_position.theta > ACT_MGR_SENSE_MIN_THETA &&
            m_current_position.theta < ACT_MGR_SENSE_MAX_THETA;
    }

    bool endstopPressed()
    {
        return digitalRead(PIN_STEPPER_ENDSTOP) == HIGH;
    }

    void sendAimPosition()
    {
        DynamixelStatus dynamixelStatus;
        dynamixelStatus = m_y_motor.goalPositionDegree(
            m_aim_position.y * ACT_MGR_Y_CONVERTER + ACT_MGR_Y_ORIGIN);
        if (dynamixelStatus != DYN_STATUS_OK) {
            m_error_code |= ACT_AX12_ERROR;
        }
        dynamixelStatus = m_theta_motor.goalPositionDegree(
            m_aim_position.theta + ACT_MGR_THETA_ORIGIN);
        if (dynamixelStatus != DYN_STATUS_OK) {
            m_error_code |= ACT_AX12_ERROR;
        }

        writeZAimPosition();
    }

    void readSensorsAndMotors()
    {
        static uint32_t last_poll_time = 0;
        static uint8_t step = 0;

        uint32_t now = micros();
        if (now - last_poll_time > ACT_MGR_POLL_PERIOD)
        {
            last_poll_time = now;
            readZCurrentPosition();
            if (step == 0)
            {
                uint16_t angle;
                DynamixelStatus dynamixelStatus = m_theta_motor.currentPositionDegree(angle);
                if (angle <= 300) {
                    m_current_position.theta = (float)angle - ACT_MGR_THETA_ORIGIN;
                }
                if (dynamixelStatus & (DYN_STATUS_OVERLOAD_ERROR | DYN_STATUS_OVERHEATING_ERROR)) {
                    m_error_code |= ACT_AX12_THETA_BLOCKED;
                }
                else if (dynamixelStatus != DYN_STATUS_OK) {
                    m_error_code |= ACT_AX12_ERROR;
                }
                step++;
            }
            else if (step == 1)
            {
                uint16_t angle;
                DynamixelStatus dynamixelStatus = m_y_motor.currentPositionDegree(angle);
                if (angle <= 300) {
                    m_current_position.y = ((float)angle - ACT_MGR_Y_ORIGIN) / ACT_MGR_Y_CONVERTER;
                }
                if (dynamixelStatus & (DYN_STATUS_OVERLOAD_ERROR | DYN_STATUS_OVERHEATING_ERROR)) {
                    m_error_code |= ACT_AX12_Y_BLOCKED;
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
                        m_raw_scan_data.push_back(
                            RawScanPoint(val, m_current_position.y + ACT_MGR_HALF_FORK_DIST));
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
                        m_raw_scan_data.push_back(
                            RawScanPoint(val, m_current_position.y - ACT_MGR_HALF_FORK_DIST));
                    }
                }
                step = 0;
            }

            if (step > 1 && !canUseSensors()) {
                step = 0;
            }
        }
    }

    void writeZAimPosition()
    {
        noInterrupts();
        m_z_current_move_origin +=
            m_z_motor.getStepsCompleted() * m_z_motor.getDirection();
        m_z_motor.stop();
        float delta_z = ACT_MGR_MICROSTEP * ACT_MGR_STEP_PER_TURN *
            (m_aim_position.z - m_current_position.z) / ACT_MGR_Z_PER_TURN;
        m_z_motor.startMove((int32_t)delta_z);
        interrupts();
    }

    void readZCurrentPosition()
    {
        noInterrupts();
        int32_t current_z_pos_step = m_z_current_move_origin +
            m_z_motor.getStepsCompleted() * m_z_motor.getDirection();
        m_current_position.z = ACT_MGR_Z_PER_TURN * (float)current_z_pos_step /
            (ACT_MGR_STEP_PER_TURN * ACT_MGR_MICROSTEP);
        interrupts();
    }

    void resetZOrigin()
    {
        noInterrupts();
        m_current_position.z = ACT_MGR_Z_MAX;
        m_z_current_move_origin = ACT_MGR_STEP_PER_TURN * ACT_MGR_MICROSTEP *
            m_current_position.z / ACT_MGR_Z_PER_TURN;
        m_z_motor.stop();
        m_z_motor.startMove(0);
        interrupts();
    }

    int computeScanData(float & y)
    {
        /* Index sensing values by their Y-coordinate */
        std::vector<int32_t> preprocess_data[ACT_MGR_SCAN_RESOLUTION];
        for (size_t i = 0; i < m_raw_scan_data.size(); i++) {
            int32_t d = m_raw_scan_data.at(i).distance;
            size_t idx = m_raw_scan_data.at(i).index;
            if (d >= 0 && idx < ACT_MGR_SCAN_RESOLUTION) {
                preprocess_data[idx].push_back(d);
            }
        }

        /* Merge sensing values with the same Y-coordinate and mark empty areas */
        int32_t scan_data[ACT_MGR_SCAN_RESOLUTION];
        for (size_t i = 0; i < ACT_MGR_SCAN_RESOLUTION; i++) {
            if (preprocess_data[i].size() == 0) {
                scan_data[i] = -1;
            }
            else {
                int32_t sum = 0;
                for (size_t j = 0; j < preprocess_data[i].size(); j++) {
                    sum += preprocess_data[i].at(j);
                }
                scan_data[i] = sum / preprocess_data[i].size();
            }
        }

        /* Fill empty areas with linear interpolation */
        for (size_t i = 0; i < ACT_MGR_SCAN_RESOLUTION; i++) {
            // todo
        }

        /* Apply moving average */
        // todo

        /* Detect puck */
        // todo

        y = 0;
        return EXIT_FAILURE;
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
    A4988 m_z_motor;
    int32_t m_z_current_move_origin; // Position, in steps, of the z-axis when the last move began
    uint32_t m_composed_move_step;
    uint32_t m_move_start_time; // ms
    bool m_scan_enabled;

    struct RawScanPoint {
        RawScanPoint(SensorValue v, float y)
        {
            if (v == OBSTACLE_TOO_CLOSE) {
                distance = ACT_MGR_SENSOR_MIN;
            }
            else if (v == NO_OBSTACLE) {
                distance = ACT_MGR_SENSOR_MAX;
            }
            else if (v > ACT_MGR_SENSOR_MAX || v < ACT_MGR_SENSOR_MIN) {
                distance = -1;
            }
            else {
                distance = v;
            }
            index = round((constrain(y, ACT_MGR_Y_MIN, ACT_MGR_Y_MAX) - ACT_MGR_Y_MIN) *
                (ACT_MGR_SCAN_RESOLUTION - 1) / (ACT_MGR_Y_MAX - ACT_MGR_Y_MIN));
        }
        size_t index;
        int32_t distance;
    };
    std::vector<RawScanPoint> m_raw_scan_data;
};

#endif
