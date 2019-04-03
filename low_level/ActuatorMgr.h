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

    void mainLoopControl() {}
    void interruptControl() {}

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
        left = m_left_sensor_value;
        right = m_right_sensor_value;
    }

    void appendSensorsValuesToVect(std::vector<uint8_t> & output) const
    {
        Serializer::writeInt(m_left_sensor_value, output);
        Serializer::writeInt(m_right_sensor_value, output);
    }

    /* Mouvement control */
    void stop();
    void goToHome();
    void goToPosition(const ActuatorPosition &p);
    void scanPuck();

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

    bool aimPositionReached() const
    {
        return m_current_position.isEqualTo(m_aim_position);
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
};

#endif
