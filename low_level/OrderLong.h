#ifndef _ORDERLONG_h
#define _ORDERLONG_h

#include <vector>
#include "OrderMacros.h"
#include "CommunicationServer.h"
#include "MotionControlSystem.h"
#include "ActuatorMgr.h"

class OrderLong
{
public:
    OrderLong() :
        inputSize(0),
        motionControlSystem(MotionControlSystem::Instance()),
        actuatorMgr(ActuatorMgr::Instance()),
        finished(true)
    {}

    /* Lancement de l'ordre long. L'argument correspond à un input. */
    void launch(const std::vector<uint8_t> &arg)
    {
        if (inputSize == VARIABLE_INPUT_SIZE || arg.size() == inputSize) {
            finished = false;
            _launch(arg);
        }
        else {
            Server.printf_err("OrderLong: wrong number of arguments\n");
        }
    }

    /* Méthode exécutée en boucle durant l'exécution de l'odre. */
    virtual void onExecute() = 0;

    /* Méthode indiquant si l'odre long a fini son exécution ou non. */
    bool isFinished()
    {
        return finished;
    }

    /* Méthode à appeler une fois que l'odre est terminé.
     * L'argument est un output, il correspond au contenu du EXECUTION_END. */
    virtual void terminate(std::vector<uint8_t> &output) = 0;

protected:
    virtual void _launch(const std::vector<uint8_t> &input) = 0;

    enum JumperState {
        WAIT_FOR_INSERTION,
        WAIT_FOR_REMOVAL,
        WAIT_FOR_DEBOUNCE_TIMER
    };

    std::size_t inputSize;
    MotionControlSystem &motionControlSystem;
    ActuatorMgr &actuatorMgr;
    bool finished;
};


#endif
