#pragma once

#include <Printable.h>

class PID : public Printable
{
public:

    PID(volatile float const& input, volatile float& output,
        volatile float const& setPoint, const float freqAsserv);

    void compute();
    void setTunings(float kp, float ki, float kd);
    void setOutputLimits(float min, float max);
    void resetDerivativeError();
    void resetIntegralError();

    float getKp() const { return kp; }
    float getKi() const { return ki; }
    float getKd() const { return kd; }
    float getError() const { return pre_error; }
    float getDerivativeError() const { return derivative; }
    float getIntegralErrol() const { return integral; }

    size_t printTo(Print& p) const;

private:
    float kp;
    float ki;
    float kd;

    volatile float const& input;
    volatile float& output;
    volatile float const& setPoint;

    float outMin, outMax;

    float pre_error;
    float derivative;
    float integral;

    const float callFreq;   // s^-1
};
