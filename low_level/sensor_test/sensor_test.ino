/*
    Name:       sensor_test.ino
    Created:	27/03/2019 18:41:37
    Author:     sg-msi\Sylvain
*/

#include <Wire.h>
#include "ToF_sensor.h"

/* ToF sensors */
#define PIN_EN_TOF_AVG          24
#define PIN_EN_TOF_AVD          35
#define PIN_EN_TOF_FOURCHE_AVG  25
#define PIN_EN_TOF_FOURCHE_AVD  36
#define PIN_EN_TOF_FLAN_ARG     21
#define PIN_EN_TOF_FLAN_ARD     16
#define PIN_EN_TOF_ARG          22
#define PIN_EN_TOF_ARD          23

#define TOF_LR_MIN_RANGE 5
#define TOF_LR_MAX_RANGE 3000

ToF_longRange tof_avg(42, PIN_EN_TOF_AVG, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "AVG", &Serial);
ToF_longRange tof_avd(44, PIN_EN_TOF_AVD, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "AVD", &Serial);
ToF_longRange tof_farg(47, PIN_EN_TOF_FLAN_ARG, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "FlanARG", &Serial);
ToF_longRange tof_fard(48, PIN_EN_TOF_FLAN_ARD, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "FlanARD", &Serial);
ToF_longRange tof_arg(49, PIN_EN_TOF_ARG, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "ARG", &Serial);
ToF_longRange tof_ard(50, PIN_EN_TOF_ARD, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "ARD", &Serial);

void setup()
{
    delay(1000);
    Wire.begin();
    tof_avg.powerON();
    //tof_farg.powerON();
}

void loop()
{
    uint32_t t1, t2;
    uint16_t val;
    uint8_t status;

    t1 = micros();
    val = tof_avg.vlSensor.readRangeContinuousMillimeters(status);
    t2 = micros();
    //Serial.println(t2 - t1);
    delay(10);

    //Serial.print("\t");
    //if (val < 2000) {
    //    Serial.print(status);
    //    Serial.print("\t");
    //    Serial.println(val);
    //}
}
