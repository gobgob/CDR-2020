#ifndef DASHBOARD_h
#define DASHBOARD_h

#include <Printable.h>
#include <Adafruit_LEDBackpack.h>
#include "Config.h"

#define DASHBOARD_UPDATE_PERIOD 50  // ms


class Dashboard : public Singleton<Dashboard>, public Printable
{
public:
    Dashboard()
    {

    }

    void init()
    {
        display.begin(0x70);
        display.clear();
        display.println(0);
        display.writeDisplay();
    }

    void update()
    {

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

    }

    void setScore(int32_t score)
    {

    }

    void setErrorLevel(ErrorLevel err_level)
    {

    }

    size_t printTo(Print& p) const
    {
        return 0;
    }

private:
    Adafruit_7segment display;

};


#endif
