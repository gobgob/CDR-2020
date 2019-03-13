#ifndef _ORDERLONG_h
#define _ORDERLONG_h

#include <vector>
#include "Serializer.h"
#include "MotionControlSystem.h"
#include "MoveState.h"
#include "CommunicationServer.h"
#include "ActuatorMgr.h"
#include "ContextualLightning.h"
#include "SensorsMgr.h"
#include "Singleton.h"

class OrderLong
{
public:
    OrderLong() :
        motionControlSystem(MotionControlSystem::Instance()),
        //slaveActuator(SlaveActuator::Instance()),
        //slaveSensorLed(SlaveSensorLed::Instance()),
        finished(true)
    {}

    void launch(const std::vector<uint8_t> & arg)
    {
        finished = false;
        _launch(arg);
    }

    /* Lancement de l'ordre long. L'argument correspond à un input (NEW_ORDER). */
    virtual void _launch(const std::vector<uint8_t> &) = 0;

    /* Méthode exécutée en boucle durant l'exécution de l'odre. */
    virtual void onExecute() = 0;

    /* Méthode indiquant si l'odre long a fini son exécution ou non. */
    bool isFinished()
    {
        return finished;
    }

    /* Méthode à appeler une fois que l'odre est terminé. L'argument est un output, il correspond au contenu du EXECUTION_END. */
    virtual void terminate(std::vector<uint8_t> &) = 0;

protected:
    MotionControlSystem & motionControlSystem;
    //SlaveActuator & slaveActuator;
    //SlaveSensorLed & slaveSensorLed;
    bool finished;
};


// ### Définition des ordres longs ###

/*
class Rien : public OrderLong, public Singleton<Rien>
{
public:
    Rien() {}
    void _launch(const std::vector<uint8_t> & input)
    {
        if (input.size() == EXPECTED_SIZE)
        {
            // process input
        }
        else
        {
            Server.printf_err("Rien: wrong number of arguments\n");
        }
    }
    void onExecute()
    {
        
    }
    void terminate(std::vector<uint8_t> & output)
    {

    }
};
//*/


class FollowTrajectory : public OrderLong, public Singleton<FollowTrajectory>
{
public:
    FollowTrajectory() { status = MOVE_OK; }
    void _launch(const std::vector<uint8_t> & input)
    {
        if (input.size() == 0)
        {
            Server.printf(SPY_ORDER, "FollowTrajectory\n");
            motionControlSystem.followTrajectory();
        }
        else
        {
            Server.printf_err("FollowTrajectory: wrong number of arguments\n");
        }
    }
    void onExecute()
    {
        if (!motionControlSystem.isMovingToDestination())
        {
            status = motionControlSystem.getMoveStatus();
            finished = true;
        }
    }
    void terminate(std::vector<uint8_t> & output)
    {
        Server.printf(SPY_ORDER, "End FollowTrajectory with status %u\n", status);
        Serializer::writeInt((int32_t)status, output);
    }

private:
    MoveStatus status;
};


class Stop : public OrderLong, public Singleton<Stop>
{
public:
    Stop() {}
    void _launch(const std::vector<uint8_t> & input)
    {
        if (input.size() == 0)
        {
            Server.printf(SPY_ORDER, "Stop");
            motionControlSystem.stop_and_clear_trajectory();
        }
        else
        {
            Server.printf_err("Stop: wrong number of arguments\n");
        }
    }
    void onExecute()
    {
        if (!motionControlSystem.isMovingToDestination())
        {
            finished = true;
        }
    }
    void terminate(std::vector<uint8_t> & output) {}
};


class WaitForJumper : public OrderLong, public Singleton<WaitForJumper>
{
public:
    WaitForJumper()
    {
        pinMode(PIN_GET_JUMPER, INPUT_PULLUP);
    }
    void _launch(const std::vector<uint8_t> & input)
    {
        Server.printf(SPY_ORDER, "WaitForJumper\n");
        state = WAIT_FOR_INSERTION;
        debounceTimer = 0;
    }
    void onExecute()
    {
        uint8_t jumperDetected = digitalRead(PIN_GET_JUMPER);
        switch (state)
        {
        case WaitForJumper::WAIT_FOR_INSERTION:
            if (jumperDetected)
            {
                state = WAIT_FOR_REMOVAL;
                //slaveSensorLed.setLightOn((uint8_t)SlaveSensorLed::FLASHING);
            }
            break;
        case WaitForJumper::WAIT_FOR_REMOVAL:
            if (!jumperDetected)
            {
                state = WAIT_FOR_DEBOUNCE_TIMER;
                debounceTimer = millis();
            }
            break;
        case WaitForJumper::WAIT_FOR_DEBOUNCE_TIMER:
            if (jumperDetected)
            {
                state = WAIT_FOR_REMOVAL;
            }
            else if (millis() - debounceTimer > 100)
            {
                finished = true;
            }
            break;
        default:
            break;
        }
    }
    void terminate(std::vector<uint8_t> & output) {}

private:
    enum JumperState
    {
        WAIT_FOR_INSERTION,
        WAIT_FOR_REMOVAL,
        WAIT_FOR_DEBOUNCE_TIMER
    };

    JumperState state;
    uint32_t debounceTimer;
};


class StartChrono : public OrderLong, public Singleton<StartChrono>
{
public:
    StartChrono() { chrono = 0; }
    void _launch(const std::vector<uint8_t> & input)
    {
        Server.printf(SPY_ORDER, "StartChrono");
        chrono = millis();
    }
    void onExecute()
    {
        if (millis() - chrono > 100000)
        {
            finished = true;
        }
    }
    void terminate(std::vector<uint8_t> & output)
    {
        motionControlSystem.stop_and_clear_trajectory();
        //slaveActuator.stop();
        // todo Maybe: prevent HL from giving orders
        //slaveSensorLed.setLightOn((uint8_t)(SlaveSensorLed::TURN_LEFT | SlaveSensorLed::TURN_RIGHT));
    }

private:
    uint32_t chrono;
};


/*
    Contrôle de l'actionneur
*/


#endif
