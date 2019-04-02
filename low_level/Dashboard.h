#ifndef DASHBOARD_h
#define DASHBOARD_h

#include <Printable.h>
#include <Adafruit_LEDBackpack.h>
#include "Config.h"
#include "MotionControlSystem.h"
#include "LedBlinker.h"
#include "Average.h"

#define DASHBOARD_UPDATE_PERIOD 50  // ms


class Dashboard : public Singleton<Dashboard>, public Printable
{
public:
    Dashboard() :
        motionControlSystem(MotionControlSystem::Instance())
    {
        pinMode(PIN_DEL_WARNING, OUTPUT);
        pinMode(PIN_DEL_ERROR, OUTPUT);
        digitalWrite(PIN_DEL_WARNING, HIGH);
        digitalWrite(PIN_DEL_ERROR, HIGH);
        displayMode = DISPLAY_SPEED;
        errorLevel = NO_ERROR;
        score = 0;
        needDisplayUpdate = false;
        warningBlinker.start();
        errorBlinker.start();
    }

    void init()
    {
        display.begin(0x70);
        display.clear();
        display.writeDigitRaw(0, 0x40);
        display.writeDigitRaw(1, 0x40);
        display.writeDigitRaw(3, 0x40);
        display.writeDigitRaw(4, 0x40);
        display.drawColon(true);
        display.writeDisplay();
        needDisplayUpdate = true;
    }

    void update()
    {
        static uint32_t lastUpdateTime = 0;
        uint32_t now = millis();
        if (now - lastUpdateTime > DASHBOARD_UPDATE_PERIOD)
        {
            lastUpdateTime = now;

            // Update LEDs
            switch (errorLevel)
            {
            case NO_ERROR:
                warningBlinker.setPeriod(600, 200);
                errorBlinker.setPeriod(1, 0);
                break;
            case WEAK_WARNING:
                warningBlinker.setPeriod(200, 600);
                errorBlinker.setPeriod(1, 0);
                break;
            case STRONG_WARNING:
                warningBlinker.setPeriod(0, 0);
                errorBlinker.setPeriod(600, 200);
                break;
            case WEAK_ERROR:
                warningBlinker.setPeriod(0, 0);
                errorBlinker.setPeriod(200, 600);
                break;
            case STRONG_ERROR:
                warningBlinker.setPeriod(200, 200);
                errorBlinker.setPeriod(100, 100);
                break;
            default:
                break;
            }
            digitalWrite(PIN_DEL_WARNING, warningBlinker.value());
            digitalWrite(PIN_DEL_ERROR, errorBlinker.value());

            // Update 7-segment display
            averageSpeed.add(motionControlSystem.getMovingSpeed());
            if (displayMode == DISPLAY_SPEED)
            {
                static int ctx = 0;
                if (ctx == 15)
                {
                    display.clear();
                    display.printFloat(abs(averageSpeed.value()), 0, 10);
                    display.writeDisplay();
                    ctx = 0;
                }
                else
                {
                    ctx++;
                }
            }
            else if (needDisplayUpdate)
            {
                display.clear();
                display.println(score);
                display.writeDisplay();
                needDisplayUpdate = false;
            }
        }
    }

    enum DisplayMode {
        DISPLAY_SPEED, DISPLAY_SCORE
    };

    enum ErrorLevel {
        NO_ERROR = 0,
        WEAK_WARNING = 1,
        STRONG_WARNING = 2,
        WEAK_ERROR = 3,
        STRONG_ERROR = 4
    };

    void setDisplayMode(DisplayMode mode)
    {
        displayMode = mode;
        needDisplayUpdate = true;
    }

    void setScore(int32_t s)
    {
        score = s;
        displayMode = DISPLAY_SCORE;
        needDisplayUpdate = true;
    }

    void setErrorLevel(ErrorLevel err_level)
    {
        errorLevel = err_level;
    }

    size_t printTo(Print& p) const
    {
        return 0;
    }

private:
    Adafruit_7segment display;
    DisplayMode displayMode;
    ErrorLevel errorLevel;
    int32_t score;
    bool needDisplayUpdate;
    const MotionControlSystem &motionControlSystem;
    LedBlinker warningBlinker;
    LedBlinker errorBlinker;
    Average<float, 8> averageSpeed;
};


#endif
