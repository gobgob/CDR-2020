#pragma once

#include <stdint.h>
#include <Encoder.h>
#include "PID.h"
#include "Average.h"
#include "VNH7100AS.h"

#define MOTOR_ENCODER_AVERAGE 50

class MotorEncoder
{
public:
    MotorEncoder(const float freqAsserv, uint8_t enc_a, uint8_t enc_b,
        uint8_t ina, uint8_t inb, uint8_t pwm, uint8_t sel, uint8_t cs,
        uint8_t write_res, uint8_t read_res);

    void setAimSpeed(float aimSpeed);
    float getCurrentSpeed() const;
    void enableSpeedControl(bool enable);
    bool isSpeedControlled() const;
    int32_t getRawTicks() const;
    void setRawPWM(float power);
    void setTunings(float kp, float ki, float kd);
    float getCurrentSense() const;
    void setMaximumCurrent(float limit);

    void sendLogs();

protected:
    void compute();
    void setAimSpeedFromInterrupt(float aimSpeed);

private:
    /* Fréquence d'appel de la méthode 'compute'. Utilisée pour le calcul des
     * vitesses. */
    const float freqAsserv; // Hz

    PID speedPID;
    volatile float motorCurrentSpeed;   // mm/s
    volatile float motorPWM;            // %
    volatile float motorSpeedSetpoint;  // mm/s
    bool speedControlled;

    Encoder encoder;
    volatile int32_t motorDeltaTicks;
    Average<float, MOTOR_ENCODER_AVERAGE> averageMotorSpeed;

    VNH7100AS motor;
};

class MotorEncoderWithInterruptAccess : public MotorEncoder
{
public:
    MotorEncoderWithInterruptAccess(const float freqAsserv, uint8_t enc_a,
        uint8_t enc_b, uint8_t ina, uint8_t inb, uint8_t pwm, uint8_t sel,
        uint8_t cs, uint8_t write_res, uint8_t read_res) :
        MotorEncoder(freqAsserv, enc_a, enc_b, ina, inb, pwm, sel, cs,
            write_res, read_res)
    {}

    using MotorEncoder::compute;
    using MotorEncoder::setAimSpeedFromInterrupt;
};
