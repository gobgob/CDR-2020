#ifndef _SENSORS_MGR_h
#define _SENSORS_MGR_h

#include <Printable.h>
#include <ToF_sensor.h>
#include "Config.h"
#include "Serializer.h"

#define SENSOR_UPDATE_PERIOD    5000 // �s
#define NB_SENSORS              6
#define TOF_SR_MIN_RANGE        18
#define TOF_SR_MAX_RANGE        200
#define TOF_LR_MIN_RANGE        30
#define TOF_LR_MAX_RANGE        700
#define TOF_LR_MEDIAN_SIZE      3


class SensorsMgr : public Printable, public Singleton<SensorsMgr>
{
public:
    SensorsMgr()
    {
        sensors[AVG] = new ToF_longRange(I2C_ADDR_TOF_AVG, PIN_EN_TOF_AVG,
            TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "AVG", &Serial);
        sensors[AVD] = new ToF_longRange(I2C_ADDR_TOF_AVD, PIN_EN_TOF_AVD,
            TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "AVD", &Serial);
        sensors[FARG] = new ToF_longRange(I2C_ADDR_TOF_FLAN_ARG, PIN_EN_TOF_FLAN_ARG,
            TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "FlanARG", &Serial);
        sensors[FARD] = new ToF_longRange(I2C_ADDR_TOF_FLAN_ARD, PIN_EN_TOF_FLAN_ARD,
            TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "FlanARD", &Serial);
        sensors[ARG] = new ToF_longRange(I2C_ADDR_TOF_ARG, PIN_EN_TOF_ARG,
            TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "ARG", &Serial);
        sensors[ARD] = new ToF_longRange(I2C_ADDR_TOF_ARD, PIN_EN_TOF_ARD,
            TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "ARD", &Serial);

        members_allocated = true;
        for (size_t i = 0; i < NB_SENSORS; i++)
        {
            sensorsValues[i] = (SensorValue)SENSOR_DEAD;
            if (sensors[i] == nullptr)
            {
                members_allocated = false;
            }
        }
    }

    int init()
    {
        if (!members_allocated) {
            return EXIT_FAILURE;
        }
        int ret = EXIT_SUCCESS;
        for (size_t i = 0; i < NB_SENSORS; i++)
        {
            if (sensors[i]->powerON() != EXIT_SUCCESS)
            {
                ret = EXIT_FAILURE;
            }
        }
        return ret;
    }

    void update(int moving_dir = 0)
    {
        static uint32_t lastUpdateTime = 0;
        static size_t step = 0;
        uint32_t now = micros();
        if (now - lastUpdateTime > SENSOR_UPDATE_PERIOD)
        {
            lastUpdateTime = now;
            if (moving_dir > 0) {
                sensorsValues[2] = (SensorValue)SENSOR_DEAD;
                sensorsValues[3] = (SensorValue)SENSOR_DEAD;
                sensorsValues[4] = (SensorValue)SENSOR_DEAD;
                sensorsValues[5] = (SensorValue)SENSOR_DEAD;
                if (step > 1) {
                    step = 0;
                }
            }
            else if (moving_dir < 0) {
                sensorsValues[0] = (SensorValue)SENSOR_DEAD;
                sensorsValues[1] = (SensorValue)SENSOR_DEAD;
                if (step < 2) {
                    step = 2;
                }
            }
            updateNow(step);
            step++;
            if (step >= NB_SENSORS) {
                step = 0;
            }
        }
    }

    void updateNow(size_t i)
    {
        if (!members_allocated || i >= NB_SENSORS) {
            return;
        }
        SensorValue val = sensors[i]->getMeasure();
        if (val != SENSOR_NOT_UPDATED) {
            sensorsValues[i] = val;
        }
    }

    void appendValuesToVect(std::vector<uint8_t> & output) const
    {
        for (size_t i = 0; i < NB_SENSORS; i++)
        {
            Serializer::writeInt(sensorsValues[i], output);
        }
    }

    size_t printTo(Print& p) const
    {
        size_t ret = 0;
        if (!members_allocated) {
            ret += p.println("SensorsMgr::allocation_error");
            return ret;
        }
        for (size_t i = 0; i < NB_SENSORS; i++)
        {
            ret += p.print(sensors[i]->name);
            ret += p.print("=");
            SensorValue val = sensorsValues[i];
            if (val == (SensorValue)SENSOR_DEAD)
            {
                ret += p.print("HS ");
            }
            else if (val == (SensorValue)SENSOR_NOT_UPDATED)
            {
                ret += p.print("Old ");
            }
            else if (val == (SensorValue)NO_OBSTACLE)
            {
                ret += p.print("inf ");
            }
            else if (val == (SensorValue)OBSTACLE_TOO_CLOSE)
            {
                ret += p.print("0 ");
            }
            else
            {
                ret += p.printf("%u ", val);
            }
        }
        return ret;
    }

private:
    ToF_sensor *sensors[NB_SENSORS];
    SensorValue sensorsValues[NB_SENSORS];
    bool members_allocated;

    enum Index
    {
        AVG = 0,
        AVD = 1,
        FARG = 2,
        FARD = 3,
        ARG = 4,
        ARD = 5,
    };
};


#endif
