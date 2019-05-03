#ifndef SMOKE_MGR_h
#define SMOKE_MGR_h

#include <Arduino.h>
#include "Singleton.h"
#include "Config.h"
#include "MotionControlSystem.h"

#define SMOKE_MGR_INTERRUPT_PERIOD  2000    // µs
#define SMOKE_MGR_UPDATE_PRIOD      200     // ms
#define SMOKE_OFF                   0
#define SMOKE_LOW                   40
#define SMOKE_MED                   100
#define SMOKE_MAX                   255

class SmokeMgr : public Singleton<SmokeMgr>
{
public:
    SmokeMgr() :
        m_mcs(MotionControlSystem::Instance())
    {
        m_pump_pwm = 0;
        m_smoke_pwm = 0;
        m_speed_dependent_smoke = false;
        pinMode(PIN_SMOKE_PUMP, OUTPUT);
        digitalWrite(PIN_SMOKE_PUMP, LOW);
        pinMode(PIN_SMOKE_RESISTOR, OUTPUT);
        digitalWrite(PIN_SMOKE_RESISTOR, LOW);
    }

    void update()
    {
        static uint32_t last_update_time = 0;
        uint32_t now = millis();
        if (now - last_update_time > SMOKE_MGR_UPDATE_PRIOD)
        {
            last_update_time = now;
            if (m_speed_dependent_smoke)
            {
                uint8_t smoke_pwm = (float)SMOKE_MAX * min(abs(m_mcs.getMovingSpeed()), 600.0) / 600.0;
                if (smoke_pwm > 0) {
                    m_pump_pwm = 255;
                }
                else {
                    m_pump_pwm = 0;
                }
                noInterrupts();
                m_smoke_pwm = smoke_pwm;
                interrupts();
            }
            analogWrite(PIN_SMOKE_PUMP, m_pump_pwm);
        }
    }

    void softPwmInterrupt()
    {
        static uint8_t counter = 0;
        if (counter == 0 && m_smoke_pwm > 0) {
            digitalWrite(PIN_SMOKE_RESISTOR, HIGH);
        }
        else if (counter > m_smoke_pwm) {
            digitalWrite(PIN_SMOKE_RESISTOR, LOW);
        }
        counter += 4;
    }

    void setConstantSmoke(uint8_t intensity)
    {
        intensity = min(intensity, SMOKE_MAX);
        m_speed_dependent_smoke = false;
        if (intensity > 0) {
            m_pump_pwm = 255;
        }
        else {
            m_pump_pwm = 0;
            analogWrite(PIN_SMOKE_PUMP, 0);
        }
        noInterrupts();
        m_smoke_pwm = intensity;
        interrupts();
    }

    void setSpeedDependantSmoke()
    {
        m_speed_dependent_smoke = true;
        m_pump_pwm = 0;
        noInterrupts();
        m_smoke_pwm = 0;
        interrupts();
    }

private:
    uint8_t m_pump_pwm;
    volatile uint8_t m_smoke_pwm;
    bool m_speed_dependent_smoke;
    const MotionControlSystem &m_mcs;
};

#endif
