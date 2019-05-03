#ifndef SMOKE_MGR_h
#define SMOKE_MGR_h

#include <Arduino.h>
#include "Singleton.h"
#include "Config.h"

class SmokeMgr : public Singleton<SmokeMgr>
{
public:
    SmokeMgr()
    {
        pinMode(PIN_SMOKE_PUMP, OUTPUT);
        pinMode(PIN_SMOKE_RESISTOR, OUTPUT);
    }

};

#endif
