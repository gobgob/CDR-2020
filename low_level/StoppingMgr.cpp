#include "StoppingMgr.h"
#include <Arduino.h>
#include "Utils.h"

StoppingMgr::StoppingMgr(volatile float const& speed) :
    speed(speed)
{
    epsilon = 0;
    responseTime = 0;
    stopped = false;
    moveBegin = false;
    beginTime = 0;
}

void StoppingMgr::compute()
{
    if (ABS(speed) < epsilon) {
        if (!stopped) {
            stopped = true;
            beginTime = millis();
        }
    }
    else {
        stopped = false;
        moveBegin = false;
    }
}

void StoppingMgr::moveIsStarting()
{
    stopped = false;
    moveBegin = true;
}

void StoppingMgr::setTunings(float epsilon, uint32_t responseTime)
{
    this->epsilon = epsilon;
    this->responseTime = responseTime;
}

void StoppingMgr::getTunings(float& epsilon, uint32_t& responseTime) const
{
    epsilon = this->epsilon;
    responseTime = this->responseTime;
}

bool StoppingMgr::isStopped() const
{
    if (moveBegin) {
        return stopped && millis() - beginTime > responseTime * 5;
    }
    else {
        return stopped && millis() - beginTime > responseTime;
    }
}

bool StoppingMgr::isMoveBegin() const
{
    return moveBegin;
}

size_t StoppingMgr::printTo(Print& p) const
{
    noInterrupts();
    float s = speed;
    bool is = isStopped();
    interrupts();

    return p.printf("%u_%g_%d", millis(), s, is);
}
