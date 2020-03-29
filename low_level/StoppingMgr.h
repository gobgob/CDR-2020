#pragma once

#include <Printable.h>
#include <stdint.h>

class StoppingMgr : public Printable
{
public:
    StoppingMgr(volatile float const& speed);

    void compute();
    void moveIsStarting();
    void setTunings(float epsilon, uint32_t responseTime);
    void getTunings(float& epsilon, uint32_t& responseTime) const;
    bool isStopped() const;
    bool isMoveBegin() const;

    size_t printTo(Print& p) const;

private:
    volatile float const & speed;
    
    float epsilon;
    uint32_t responseTime; // ms

    uint32_t beginTime;
    bool stopped;
    bool moveBegin;
};
