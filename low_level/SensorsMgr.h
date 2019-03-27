#ifndef _SENSORS_MGR_h
#define _SENSORS_MGR_h

#include <Printable.h>
#include <ToF_sensor.h>
#include "Config.h"
#include "Serializer.h"

#define NB_SENSORS          6
#define TOF_SR_MIN_RANGE    18
#define TOF_SR_MAX_RANGE    200
#define TOF_LR_MIN_RANGE    30
#define TOF_LR_MAX_RANGE    700
#define TOF_LR_MEDIAN_SIZE  3


class SensorsMgr : public Printable, public Singleton<SensorsMgr>
{
public:
    SensorsMgr()
    {
        sensors[0] = new ToF_longRange_med<TOF_LR_MEDIAN_SIZE>(
            42, PIN_EN_TOF_AVG, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "AVG", &Serial);
        sensors[1] = new ToF_longRange_med<TOF_LR_MEDIAN_SIZE>(
            44, PIN_EN_TOF_AVD, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "AVD", &Serial);
        sensors[2] = new ToF_longRange_med < TOF_LR_MEDIAN_SIZE>(
            47, PIN_EN_TOF_FLAN_ARG, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "FlanARG", &Serial);
        sensors[3] = new ToF_longRange_med<TOF_LR_MEDIAN_SIZE>(
            48, PIN_EN_TOF_FLAN_ARD, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "FlanARD", &Serial);
        sensors[4] = new ToF_longRange_med<TOF_LR_MEDIAN_SIZE>(
            49, PIN_EN_TOF_ARG, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "ARG", &Serial);
        sensors[5] = new ToF_longRange_med<TOF_LR_MEDIAN_SIZE>(
            50, PIN_EN_TOF_ARD, TOF_LR_MIN_RANGE, TOF_LR_MAX_RANGE, "ARD", &Serial);

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

    void update()
    {
        if (!members_allocated) {
            return;
        }
        for (size_t i = 0; i < NB_SENSORS; i++)
        {
            SensorValue val = sensors[i]->getMeasure();
            if (val != SENSOR_NOT_UPDATED) {
                sensorsValues[i] = val;
            }
        }
    }

    void update(size_t i)
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
};


#endif
