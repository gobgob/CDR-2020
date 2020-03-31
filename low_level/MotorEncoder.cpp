#include "MotorEncoder.h"
#include <Arduino.h>
#include "Serial.h"

#define MOTOR_ENCODER_TICK_TO_MM 0.1201 // mm/tick
#define MOTOR_ENCODER_OC_RECOVER_DELAY 1000 // ms
#define MOTOR_ENCODRE_MIN_SPEED 5  // mm/s

MotorEncoder::MotorEncoder(const float freqAsserv, uint8_t enc_a,
    uint8_t enc_b, uint8_t ina, uint8_t inb, uint8_t pwm, uint8_t sel,
    uint8_t cs, uint8_t write_res, uint8_t read_res) :
    freqAsserv(freqAsserv),
    speedPID(motorCurrentSpeed, motorPWM, motorSpeedSetpoint, freqAsserv),
    encoder(enc_a, enc_b),
    motor(ina, inb, pwm, sel, cs, write_res, read_res)
{
    speedPID.setOutputLimits(-100, 100);
    motorCurrentSpeed = 0;
    motorPWM = 0;
    motorSpeedSetpoint = 0;
    speedControlled = true;
    motorDeltaTicks = 0;
}

void MotorEncoder::setAimSpeed(float aimSpeed)
{
    noInterrupts();
    setAimSpeedFromInterrupt(aimSpeed);
    interrupts();
}

float MotorEncoder::getCurrentSpeed() const
{
    float ret;
    noInterrupts();
    ret = motorCurrentSpeed;
    interrupts();
    return ret;
}

void MotorEncoder::enableSpeedControl(bool enable)
{
    noInterrupts();
    speedControlled = enable;
    if (!enable) {
        motor.run(0);
    }
    interrupts();
}

bool MotorEncoder::isSpeedControlled() const
{
    return speedControlled;
}

int32_t MotorEncoder::getRawTicks() const
{
    int32_t ret;
    noInterrupts();
    ret = motorDeltaTicks;
    interrupts();
    return ret;
}

void MotorEncoder::setRawPWM(float power)
{
    if (!speedControlled) {
        noInterrupts();
        motor.run(power);
        interrupts();
    }
    else {
        Server.printf_err("MotorEncoder::setRawPWM : speed is controlled\n");
    }
}

void MotorEncoder::setTunings(float kp, float ki, float kd)
{
    noInterrupts();
    speedPID.setTunings(kp, ki, kd);
    interrupts();
}

float MotorEncoder::getCurrentSense() const
{
    float ret;
    noInterrupts();
    ret = motor.getCurrent();
    interrupts();
    return ret;
}

void MotorEncoder::setMaximumCurrent(float limit)
{
    noInterrupts();
    motor.setMaximumCurrent(limit);
    interrupts();
}

void MotorEncoder::sendLogs()
{
    Server.print(PID_SPEED, speedPID);
    noInterrupts();
    float c = motor.getCurrent();
    interrupts();
    Server.printf(BLOCKING_MGR, "%u_0_0_%g\n", millis(), c);
}

void MotorEncoder::compute()
{
    static bool overcurent = false;
    static uint32_t oc_start_time = 0;

    motorDeltaTicks = encoder.readAndReset();
    averageMotorSpeed.add((float)motorDeltaTicks * MOTOR_ENCODER_TICK_TO_MM *
        freqAsserv);
    motorCurrentSpeed = averageMotorSpeed.value();
    if (abs(motorCurrentSpeed) < MOTOR_ENCODRE_MIN_SPEED) {
        motorCurrentSpeed = 0;
    }

    motor.checkCurrent();
    if (motor.overcurrent()) {
        uint32_t now = millis();
        if (overcurent && now - oc_start_time > MOTOR_ENCODER_OC_RECOVER_DELAY) {
            overcurent = false;
            motor.clearOvercurrentFlag();
            Server.printf("MotorEncoder : return to normal operation after "
                "overcurrent incident\n");
        }

        if (!overcurent) {
            overcurent = true;
            oc_start_time = now;
            Server.printf_err("MotorEncoder : overcurrent\n");
        }
    }

    if (speedControlled) {
        /* MAJ motorPWM */
        speedPID.compute();
        motor.run(motorPWM);
    }
}

void MotorEncoder::setAimSpeedFromInterrupt(float aimSpeed)
{
    motorSpeedSetpoint = aimSpeed;
    if (aimSpeed == 0) {
        speedPID.resetIntegralError();
        speedPID.resetDerivativeError();
    }
}
