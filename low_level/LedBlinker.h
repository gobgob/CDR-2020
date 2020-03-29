#pragma once

#include <stdint.h>

class LedBlinker
{
public:
    LedBlinker()
    {
        off_duration = 0;
        on_duration = 0;
        origin_time = 0;
    }

    void setPeriod(uint32_t off_d, uint32_t on_d)
    {
        off_duration = off_d;
        on_duration = on_d;
    }

    void start()
    {
        origin_time = millis();
    }

    bool value() const
    {
        if (off_duration == 0) {
            return true;
        }
        else if (on_duration == 0) {
            return false;
        }

        uint32_t now = millis() - origin_time;
        now = now % (on_duration + off_duration);
        return now < on_duration;
    }

private:
    uint32_t off_duration;  // ms
    uint32_t on_duration;   // ms
    uint32_t origin_time;   // ms
};
