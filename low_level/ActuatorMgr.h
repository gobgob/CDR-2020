#ifndef ACTUATOR_MGR_h
#define ACTUATOR_MGR_h

#include <DynamixelAX12.h>
#include <Printable.h>
#include <vector>
#include "Serial.h"
#include "Singleton.h"
#include "Serializer.h"
#include "Config.h"
#include "Serial.h"


enum ActuatorMgrStatus
{
    ACT_STATUS_OK = 0,
};


class ActuatorMgr : public Singleton<ActuatorMgr>
{
public:
    ActuatorMgr()
    {
    }

    int init()
    {
        return ACT_STATUS_OK;
    }

    void control()
    {
    }
};

#endif
