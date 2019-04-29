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
        actuatorMgr(ActuatorMgr::Instance()),
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
    ActuatorMgr & actuatorMgr;
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
        actuatorMgr.stop();
    }

private:
    uint32_t chrono;
};


/*
    Contrôle de l'actionneur
*/

class ActuatorGoHome : public OrderLong, public Singleton<ActuatorGoHome>
{
public:
    ActuatorGoHome() {}
    void _launch(const std::vector<uint8_t> & input)
    {
        if (input.size() == 0)
        {
            Server.printf(SPY_ORDER, "ActuatorGoHome");
            if (actuatorMgr.goToHome() != EXIT_SUCCESS)
            {
                finished = true;
                ret_code = ACT_ALREADY_MOVING;
            }
            else
            {
                ret_code = ACT_OK;
            }
        }
        else
        {
            Server.printf_err("ActuatorGoHome: wrong number of arguments\n");
        }
    }
    void onExecute()
    {
        if (actuatorMgr.commandCompleted())
        {
            finished = true;
        }
    }
    void terminate(std::vector<uint8_t> & output)
    {
        ret_code |= actuatorMgr.getErrorCode();
        Serializer::writeInt(ret_code, output);
    }

private:
    ActuatorErrorCode ret_code;
};

class ActuatorGoTo : public OrderLong, public Singleton<ActuatorGoTo>
{
public:
    ActuatorGoTo() {}
    void _launch(const std::vector<uint8_t> & input)
    {
        if (input.size() == 12)
        {
            Server.printf(SPY_ORDER, "ActuatorGoTo");
            ActuatorPosition p;
            size_t index = 0;
            p.y = Serializer::readFloat(input, index);
            p.z = Serializer::readFloat(input, index);
            p.theta = Serializer::readFloat(input, index);

            if (actuatorMgr.goToPosition(p) != EXIT_SUCCESS)
            {
                finished = true;
                ret_code = ACT_ALREADY_MOVING;
            }
            else
            {
                ret_code = ACT_OK;
            }
        }
        else
        {
            Server.printf_err("ActuatorGoTo: wrong number of arguments\n");
        }
    }
    void onExecute()
    {
        if (actuatorMgr.commandCompleted())
        {
            finished = true;
        }
    }
    void terminate(std::vector<uint8_t> & output)
    {
        ret_code |= actuatorMgr.getErrorCode();
        Serializer::writeInt(ret_code, output);
    }

private:
    ActuatorErrorCode ret_code;
};

class ActuatorFindPuck : public OrderLong, public Singleton<ActuatorFindPuck>
{
public:
    ActuatorFindPuck() {}
    void _launch(const std::vector<uint8_t> & input)
    {
        if (input.size() == 0)
        {
            Server.printf(SPY_ORDER, "ActuatorFindPuck");
            if (actuatorMgr.scanPuck() != EXIT_SUCCESS)
            {
                finished = true;
                ret_code = ACT_ALREADY_MOVING;
            }
            else
            {
                ret_code = ACT_OK;
            }
        }
        else
        {
            Server.printf_err("ActuatorFindPuck: wrong number of arguments\n");
        }
    }
    void onExecute()
    {
        if (actuatorMgr.commandCompleted())
        {
            finished = true;
        }
    }
    void terminate(std::vector<uint8_t> & output)
    {
        ret_code |= actuatorMgr.getErrorCode();
        Serializer::writeInt(ret_code, output);
    }

private:
    ActuatorErrorCode ret_code;
};

class ActuatorGoToWithSpeed : public OrderLong, public Singleton<ActuatorGoToWithSpeed>
{
public:
    ActuatorGoToWithSpeed() {}
    void _launch(const std::vector<uint8_t>& input)
    {
        if (input.size() == 24)
        {
            Server.printf(SPY_ORDER, "ActuatorGoToWithSpeed");
            ActuatorPosition p;
            size_t index = 0;
            p.y = Serializer::readFloat(input, index);
            p.z = Serializer::readFloat(input, index);
            p.theta = Serializer::readFloat(input, index);
            int32_t y_speed = Serializer::readInt(input, index);
            int32_t z_speed = Serializer::readInt(input, index);
            int32_t theta_speed = Serializer::readInt(input, index);

            if (actuatorMgr.goToPosition(p, y_speed, z_speed, theta_speed) != EXIT_SUCCESS)
            {
                finished = true;
                ret_code = ACT_ALREADY_MOVING;
            }
            else
            {
                ret_code = ACT_OK;
            }
        }
        else
        {
            Server.printf_err("ActuatorGoToWithSpeed: wrong number of arguments\n");
        }
    }
    void onExecute()
    {
        if (actuatorMgr.commandCompleted())
        {
            finished = true;
        }
    }
    void terminate(std::vector<uint8_t>& output)
    {
        ret_code |= actuatorMgr.getErrorCode();
        Serializer::writeInt(ret_code, output);
    }

private:
    ActuatorErrorCode ret_code;
};

#endif
