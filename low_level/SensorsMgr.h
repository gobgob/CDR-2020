#ifndef _SENSORS_MGR_h
#define _SENSORS_MGR_h

#include <Printable.h>
#include <ToF_module.h>
#include "Config.h"
#include "Serializer.h"

#define SENSOR_UPDATE_PERIOD    5000 // µs
#define NB_SENSORS              10


class SensorsMgr : public Printable, public Singleton<SensorsMgr>
{
public:
    SensorsMgr()
    {
        sensors[AVD ] = new ToF_module(SerialToF, AVD);
        sensors[FAVD] = new ToF_module(SerialToF, FAVD);
        sensors[FARD] = new ToF_module(SerialToF, FARD);
        sensors[ARD] = new ToF_module(SerialToF, ARD);
        sensors[ARG] = new ToF_module(SerialToF, ARG);
        sensors[FARG] = new ToF_module(SerialToF, FARG);
        sensors[FAVG] = new ToF_module(SerialToF, FAVG);
        sensors[AVG] = new ToF_module(SerialToF, AVG);
        sensors[EXTG] = new ToF_module(SerialExt, EXTG);
        sensors[EXTD] = new ToF_module(SerialExt, EXTD);

        members_allocated = true;
        for (size_t i = 0; i < NB_SENSORS; i++) {
            sensorsValues[i] = (TofValue)SENSOR_DEAD;
            sensorsLastUpdateTime[i] = 0;
            if (sensors[i] == nullptr) {
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
        OneWireStatus com_status;
        for (size_t i = 0; i < NB_SENSORS; i++) {
            com_status = sensors[i]->init();
            if (com_status != OW_STATUS_OK) {
                ret = EXIT_FAILURE;
            }
            else if (sensors[i]->statusReturnLevel() == 0) {
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

        if (now - lastUpdateTime > SENSOR_UPDATE_PERIOD) {
            lastUpdateTime = now;

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

        uint32_t now = millis();
        TofValue val = sensors[i]->readRange();

        bool needSensorReset = false;
        if (val != SENSOR_NOT_UPDATED) {
            sensorsValues[i] = val;
            sensorsLastUpdateTime[i] = now;
        }
        else if (now - sensorsLastUpdateTime[i] > 100) {
            Server.printf_err("SensorsMgr::updateNow(%u) sensor didn't perform measurements for more than 100ms\n", i);
            needSensorReset = true;
        }

        if (needSensorReset) {
            Server.printf("Attempting to restart sensor #%u\n", i);
            sensors[i]->softReset();
        }
    }

    uint32_t getLastUpdateTime(size_t i) const
    {
        if (i >= NB_SENSORS) {
            return 0;
        }
        return millis() - sensorsLastUpdateTime[i];
    }

    void appendValuesToVect(std::vector<uint8_t> &output) const
    {
        for (size_t i = 0; i < NB_SENSORS; i++) {
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

        for (size_t i = 0; i < NB_SENSORS; i++) {
            ret += p.print(names[i]);
            ret += p.print("=");

            switch (sensorsValues[i]) {
                case (TofValue)SENSOR_DEAD:
                    ret += p.print("HS ");
                    break;
                case (TofValue)SENSOR_NOT_UPDATED:
                    ret += p.print("Old ");
                    break;
                case (TofValue)NO_OBSTACLE:
                    ret += p.print("inf ");
                    break;
                case (TofValue)OBSTACLE_TOO_CLOSE:
                    ret += p.print("0 ");
                    break;
                default:
                    ret += p.printf("%d ", sensorsValues[i]);
                    break;
            }
        }
        ret += p.println();
        return ret;
    }

private:
    ToF_module* sensors[NB_SENSORS];
    TofValue sensorsValues[NB_SENSORS];
    uint32_t sensorsLastUpdateTime[NB_SENSORS];
    bool members_allocated;

    enum Index {
        AVD = 0,    // Avant droit
        FAVD = 1,   // Flan avant droit
        FARD = 2,   // Flan arrière droit
        ARD = 3,    // Arrière droit
        ARG = 4,    // Arrière gauche
        FARG = 5,   // Flan arrière gauche
        FAVG = 6,   // Flan avant gauche
        AVG = 7,    // Avant gauche
        EXTG = 8,   // Extérieur gauche (robot secondaire)
        EXTD = 9,   // Extérieur droit (robot secondaire)
    };

    const char* names[NB_SENSORS] = {
        "AVD",
        "FAVD",
        "FARD",
        "ARD",
        "ARG",
        "FARG",
        "FAVG",
        "AVG",
        "EXTG",
        "EXTD",
    };
};


#endif
