#ifndef ACTUATOR_MGR_h
#define ACTUATOR_MGR_h

#include <A4988.h>
#include <ToF_sensor.h>
#include <Printable.h>
#include "Singleton.h"
#include "Config.h"

#define ACTUATOR_MGR_INTERRUPT_PERIOD   1000 // µs
#define ACTUATOR_MGR_MOVE_TIMEOUT       8000 // ms
#define ACTUATOR_MGR_Y_TOLERANCE        1.5  // mm
#define ACTUATOR_MGR_Z_TOLERANCE        1.0  // mm
#define ACTUATOR_MGR_THETA_TOLERANCE    5    // deg


enum ActuatorStatus
{
    ACT_IDLE,
    ACT_MOVING,
    ACT_BLOCKED,
    ACT_STOP_REQUESTED,
    ACT_NO_PUCK
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
    ActuatorMgr()
    {
        attachInterrupt(PIN_STEPPER_ENDSTOP, endstopInterrupt, CHANGE);
    }

    void mainLoopControl() {}
    void interruptControl() {}
    ActuatorStatus getStatus() const;
    void getSensorsValues(SensorValue &left, SensorValue &right);

    /* Mouvement control */
    void stop();
    void goToHome();
    void goToPosition(const ActuatorPosition &p);
    void scanPuck();

private:
    static void endstopInterrupt() {}
    bool aimPositionReached() const
    {
        return m_current_position.isEqualTo(m_aim_position);
    }

    ActuatorPosition m_current_position;
    ActuatorPosition m_aim_position;

    ToF_shortRange m_left_sensor;
    ToF_shortRange m_right_sensor;
};

#endif
