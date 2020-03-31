#include "VNH7100AS.h"
#include <Arduino.h>

#define DEFAULT_MAX_CURRENT 5.0f // Amp
#define PERCENT_TO_AMPS 0.15f // Very routh estimate
#define OVERCURRENT_DELAY 500  // ms

VNH7100AS::VNH7100AS(uint8_t ina, uint8_t inb, uint8_t pwm, uint8_t sel,
    uint8_t cs, uint8_t write_res, uint8_t read_res) :
    pin_ina(ina),
    pin_inb(inb),
    pin_pwm(pwm),
    pin_sel(sel),
    pin_cs(cs),
    max_analog_write((1 << write_res) - 1),
    max_analog_read((1 << read_res) - 1)
{
    breaks = true;
    overcurrentFlag = false;
    moving = false;
    lastCrrentMeasure = 0;
    maxCurrent = DEFAULT_MAX_CURRENT;
    lastCurrentOkTime = 0;
    pinMode(pin_ina, OUTPUT);
    pinMode(pin_inb, OUTPUT);
    pinMode(pin_pwm, OUTPUT);
    pinMode(pin_sel, OUTPUT);
    pinMode(pin_cs, INPUT);
    standby();
}

VNH7100AS::~VNH7100AS()
{
    pinMode(pin_ina, INPUT);
    pinMode(pin_inb, INPUT);
    pinMode(pin_pwm, INPUT);
    pinMode(pin_sel, INPUT);
    pinMode(pin_cs, INPUT);
}

void VNH7100AS::run(float power)
{
    if (overcurrentFlag) {
        return;
    }

    int pwm = round(constrain(power, -100, 100) *
        (float)max_analog_write / 100.0f);
    bool forward = pwm > 0;
    moving = pwm != 0;

    if (moving) {
        digitalWrite(pin_ina, !forward);
        digitalWrite(pin_inb, forward);
        digitalWrite(pin_sel, !forward);
    }
    else {
        digitalWrite(pin_sel, HIGH);
        digitalWrite(pin_ina, breaks);
        digitalWrite(pin_inb, breaks);
    }
    analogWrite(pin_pwm, abs(pwm));
}

void VNH7100AS::checkCurrent()
{
    if (moving) {
        float current = currentConversion((float)analogRead(pin_cs) *
            100.0f / (float)max_analog_read);
        avgCurrent.add(current);
    }
    else {
        avgCurrent.reset();
    }

    lastCrrentMeasure = avgCurrent.value();

    uint32_t now = millis();
    if (lastCrrentMeasure > maxCurrent) {
        if (now - lastCurrentOkTime > OVERCURRENT_DELAY) {
            overcurrentFlag = true;
            standby();
        }
    }
    else {
        lastCurrentOkTime = now;
    }
}

void VNH7100AS::enableBreaks(bool enable)
{
    breaks = enable;
}

void VNH7100AS::standby()
{
    moving = false;
    digitalWrite(pin_sel, LOW);
    digitalWrite(pin_ina, LOW);
    digitalWrite(pin_inb, LOW);
    analogWrite(pin_pwm, LOW);
}

void VNH7100AS::setMaximumCurrent(float limit)
{
    if (limit <= 0) {
        return;
    }
    maxCurrent = limit;
}

float VNH7100AS::getCurrent() const
{
    return lastCrrentMeasure;
}

bool VNH7100AS::overcurrent() const
{
    return overcurrentFlag;
}

void VNH7100AS::clearOvercurrentFlag()
{
    overcurrentFlag = false;
}

float VNH7100AS::currentConversion(float measurement)
{
    return measurement * PERCENT_TO_AMPS;
}
