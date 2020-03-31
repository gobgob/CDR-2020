#pragma once

#include <stdint.h>
#include "Average.h"

/* H-bridge driver */
class VNH7100AS
{
public:
    VNH7100AS(uint8_t ina, uint8_t inb, uint8_t pwm, uint8_t sel, uint8_t cs,
        uint8_t write_res, uint8_t read_res);
    ~VNH7100AS();

    /* Run the motor with the given algebric power in % */
    void run(float power);

    /* Perform one current measurement, if the current is higher than the limit
     * the overcurrent flag will be raised and subsequent calls to run will
     * have no effect until clearOvercurrentFlag is called. */
    void checkCurrent();

    /* Choose between breaking and free-rolling modes for when the power is set
     * to 0 */
    void enableBreaks(bool enable);

    /* Set the H-bridge in standby mode. Call run to get out of standby. */
    void standby();

    /* Set the maximum current limit. Unit: Amps */
    void setMaximumCurrent(float limit);

    /* Get the last current measured, if the motor is not moving the value will
     * be 0. Unit: Amps */
    float getCurrent() const;

    /* Return true if the current limit was reached */
    bool overcurrent() const;

    /* Return to normal operation after an overcurrent */
    void clearOvercurrentFlag();

private:
    static float currentConversion(float measurement);

    const uint8_t pin_ina;  // H-bridge input A
    const uint8_t pin_inb;  // H-bridge input B
    const uint8_t pin_pwm;  // H-bridge PWM input
    const uint8_t pin_sel;  // H-bridge sensing selection
    const uint8_t pin_cs;   // H-bridge current sense
    const uint32_t max_analog_write;    // Analog Write range
    const uint32_t max_analog_read;     // Analog Read range

    bool breaks;
    bool overcurrentFlag;
    bool moving;
    float lastCrrentMeasure;
    float maxCurrent;
    uint32_t lastCurrentOkTime;

    Average<float, 50> avgCurrent;
};
