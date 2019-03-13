#ifndef ACTUATOR_MGR_h
#define ACTUATOR_MGR_h

#include <A4988.h>
#include <ToF_sensor.h>
#include "Singleton.h"
#include "Config.h"

enum ActuatorStatus
{
    ACT_IDLE, ACT_MOVING, ACT_BLOCKED
};


class ActuatorMgr : public Singleton<ActuatorMgr>
{
public:
    ActuatorMgr() {}

    void mainLoopControl() {}
    void interruptControl() {}
    ActuatorStatus getStatus() const;
    void setDefaultTimeout();
    void getSensorsValues(SensorValue &left, SensorValue &right);

    /* Mouvement control */
    void stop();
    void goToHome();
    // todo
};

#endif
