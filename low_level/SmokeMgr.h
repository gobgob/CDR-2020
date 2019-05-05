#ifndef SMOKE_MGR_h
#define SMOKE_MGR_h

#include <Arduino.h>
#include "Singleton.h"
#include "Config.h"
#include "MotionControlSystem.h"
#include "CommunicationServer.h"

#define SMOKE_MGR_UPDATE_PRIOD      1000    // ms
#define SMOKE_OFF                   0       // pwm
#define SMOKE_ON                    32762   // pwm

class SmokeMgr : public Singleton<SmokeMgr>
{
public:
    SmokeMgr()
    {
        m_enabled = false;
        m_smoke_on = false;
        m_last_update_time = 0;
        pinMode(PIN_SMOKE_PUMP, OUTPUT);
        digitalWrite(PIN_SMOKE_PUMP, LOW);
        pinMode(PIN_SMOKE_RESISTOR, OUTPUT);
        digitalWrite(PIN_SMOKE_RESISTOR, LOW);
    }

    void update()
    {
        uint32_t now = millis();
        if (now - m_last_update_time > SMOKE_MGR_UPDATE_PRIOD)
        {
            m_last_update_time = now;
            m_smoke_on = !m_smoke_on;
            if (m_enabled && m_smoke_on) {
                analogWrite(PIN_SMOKE_RESISTOR, SMOKE_ON);
            }
            else {
                analogWrite(PIN_SMOKE_RESISTOR, SMOKE_OFF);
            }
            digitalWrite(PIN_SMOKE_PUMP, m_enabled && m_smoke_on);
        }
    }

    void enableSmoke(bool enable)
    {
        m_enabled = enable;
        m_last_update_time = 0;
        m_smoke_on = false;
    }

private:
    bool m_enabled;
    bool m_smoke_on;
    uint32_t m_last_update_time;
};

#endif
