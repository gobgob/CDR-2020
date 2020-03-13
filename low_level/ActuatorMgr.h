#ifndef ACTUATOR_MGR_h
#define ACTUATOR_MGR_h

#include <DynamixelAX12.h>
#include <Printable.h>
#include <vector>
#include "Serial.h"
#include "Singleton.h"
#include "Serializer.h"
#include "Config.h"
#include "CommunicationServer.h"


class ActuatorMgr : public Singleton<ActuatorMgr>
{
public:
    ActuatorMgr()
    {
    }

    int init()
    {
        return 0;
    }

    void control()
    {
    }
};

#endif
