#ifndef _ORDERIMMEDIATE_h
#define _ORDERIMMEDIATE_h

#include <vector>
#include "OrderMacros.h"
#include "CommunicationServer.h"
#include "MotionControlSystem.h"
#include "DirectionController.h"
#include "ActuatorMgr.h"
#include "SensorsMgr.h"


class OrderImmediate
{
public:
    OrderImmediate() :
        inputSize(0),
        motionControlSystem(MotionControlSystem::Instance()),
        directionController(DirectionController::Instance()),
        actuatorMgr(ActuatorMgr::Instance()),
        sensorMgr(SensorsMgr::Instance())
    {}

    /* Méthode exécutant l'ordre immédiat.
     * Le premier arument est l'input, le second l'output. */
    void execute(const std::vector<uint8_t> &input,
        std::vector<uint8_t> &output)
    {
        if (inputSize == VARIABLE_INPUT_SIZE || input.size() == inputSize) {
            _execute(input, output);
        }
        else {
            Server.printf_err("OrderImmediate: wrong number of arguments\n");
        }
    }

protected:
    virtual void _execute(const std::vector<uint8_t> &input,
        std::vector<uint8_t> &output) = 0;

    std::size_t inputSize;
    MotionControlSystem &motionControlSystem;
    DirectionController &directionController;
    ActuatorMgr &actuatorMgr;
    const SensorsMgr &sensorMgr;
};


#endif
