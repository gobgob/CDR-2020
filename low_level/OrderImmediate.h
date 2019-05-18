#ifndef _ORDERIMMEDIATE_h
#define _ORDERIMMEDIATE_h

#include <vector>
#include "Serializer.h"
#include "MotionControlSystem.h"
#include "TrajectoryFollower.h"
#include "DirectionController.h"
#include "MoveState.h"
#include "Position.h"
#include "TrajectoryPoint.h"
#include "MotionControlTunings.h"
#include "CommunicationServer.h"
#include "ActuatorMgr.h"
#include "Dashboard.h"
#include "ContextualLightning.h"
#include "Singleton.h"
#include "SmokeMgr.h"
#include "SensorsMgr.h"


class OrderImmediate
{
public:
    OrderImmediate() :
        motionControlSystem(MotionControlSystem::Instance()),
        directionController(DirectionController::Instance()),
        dashboard(Dashboard::Instance()),
        contextualLightning(ContextualLightning::Instance()),
        actuatorMgr(ActuatorMgr::Instance()),
        smokeMgr(SmokeMgr::Instance()),
        sensorMgr(SensorsMgr::Instance())
    {}

    /*
    Méthode exécutant l'ordre immédiat.
    L'argument correspond à la fois à l'input et à l'output de l'ordre, il sera modifié par la méthode.
    */
    virtual void execute(std::vector<uint8_t> &) = 0;

protected:
    enum Color
    {
        VIOLET = 0x00,
        JAUNE = 0x01,
        UNKNOWN = 0x02
    };

    MotionControlSystem & motionControlSystem;
    DirectionController & directionController;
    Dashboard & dashboard;
    ContextualLightning & contextualLightning;
    ActuatorMgr & actuatorMgr;
    SmokeMgr & smokeMgr;
    const SensorsMgr & sensorMgr;
};


// ### Définition des ordres à réponse immédiate ###

/*
class Rien : public OrderImmediate, public Singleton<Rien>
{
public:
    Rien() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == EXPECTED_SIZE)
        {
            // Read input
            // Process command
            io.clear();
            // Write output
        }
        else
        {
            Server.printf_err("Rien: wrong number of arguments\n");
            io.clear();
        }
    }
};
//*/

/*
    Ne fait rien, mais indique que le HL est vivant !
*/
class Ping : public OrderImmediate, public Singleton<Ping>
{
public:
    Ping() {}

    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            io.clear();
            Serializer::writeInt(0, io);
        }
        else
        {
            Server.printf_err("Ping: wrong number of arguments\n");
            io.clear();
        }
    }
};


class GetColor : public OrderImmediate, public Singleton<GetColor>
{
public:
    GetColor()
    {
        pinMode(PIN_GET_COLOR, INPUT);
        pinMode(PIN_GET_JUMPER, INPUT_PULLUP);
    }
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            io.clear();

            uint8_t jumperDetected = digitalRead(PIN_GET_JUMPER);
            Color color = UNKNOWN;
            if (jumperDetected)
            {
                if (digitalRead(PIN_GET_COLOR))
                {
                    color = VIOLET;
                }
                else
                {
                    color = JAUNE;
                }
            }
            Serializer::writeEnum((uint8_t)color, io);
        }
        else
        {
            Server.printf_err("GetColor: wrong number of arguments\n");
            io.clear();
        }
    }
};


class EditPosition : public OrderImmediate, public Singleton<EditPosition>
{
public:
    EditPosition() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 12)
        {
            size_t index = 0;
            int32_t x = Serializer::readInt(io, index);
            int32_t y = Serializer::readInt(io, index);
            float angle = Serializer::readFloat(io, index);
            Position p = motionControlSystem.getPosition();
            p.x += (float)x;
            p.y += (float)y;
            p.setOrientation(p.orientation + (float)angle);
            Server.printf(SPY_ORDER, "EditPosition, result=");
            Server.print(SPY_ORDER, p);
            motionControlSystem.setPosition(p);
            io.clear();
        }
        else
        {
            Server.printf_err("EditPosition: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetPosition : public OrderImmediate, public Singleton<SetPosition>
{
public:
    SetPosition() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 12)
        {
            size_t index = 0;
            int32_t x = Serializer::readInt(io, index);
            int32_t y = Serializer::readInt(io, index);
            float angle = Serializer::readFloat(io, index);
            Position p((float)x, (float)y, angle);
            Server.printf(SPY_ORDER, "SetPosition");
            Server.print(SPY_ORDER, p);
            motionControlSystem.setPosition(p);
            io.clear();
        }
        else
        {
            Server.printf_err("SetPosition: wrong number of arguments\n");
            io.clear();
        }
    }
};


class AppendToTraj : public OrderImmediate, public Singleton<AppendToTraj>
{
public:
    AppendToTraj() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        uint8_t ret = TRAJECTORY_EDITION_FAILURE;
        if (io.size() % 22 == 0)
        {
            for (size_t i = 0; i < io.size(); i += 22)
            {
                size_t index = i;
                int32_t x = Serializer::readInt(io, index);
                int32_t y = Serializer::readInt(io, index);
                float angle = Serializer::readFloat(io, index);
                float curvature = Serializer::readFloat(io, index);
                float speed = Serializer::readFloat(io, index);
                bool stopPoint = Serializer::readBool(io, index);
                bool endOfTraj = Serializer::readBool(io, index);
                Position p((float)x, (float)y, angle);
                TrajectoryPoint trajPoint(p, curvature, speed, stopPoint, endOfTraj);
                Server.printf(SPY_ORDER, "AppendToTraj: ");
                Server.println(SPY_ORDER, trajPoint);
                ret = motionControlSystem.appendToTrajectory(trajPoint);
                if (ret != TRAJECTORY_EDITION_SUCCESS)
                {
                    motionControlSystem.stop_and_clear_trajectory();
                    Server.printf_err("AppendToTraj: TRAJECTORY_EDITION_FAILURE");
                    break;
                }
            }
        }
        else
        {
            Server.printf_err("AppendToTraj: wrong number of arguments\n");
        }
        io.clear();
        Serializer::writeEnum(ret, io);
    }
};


class EditTraj : public OrderImmediate, public Singleton<EditTraj>
{
public:
    EditTraj() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        uint8_t ret = TRAJECTORY_EDITION_FAILURE;
        if (io.size() > 0 && (io.size() - 1) % 22 == 0)
        {
            size_t index = 0;
            size_t trajIndex = Serializer::readUInt(io, index);

            for (size_t i = 1; i < io.size(); i += 22)
            {
                index = i;
                int32_t x = Serializer::readInt(io, index);
                int32_t y = Serializer::readInt(io, index);
                float angle = Serializer::readFloat(io, index);
                float curvature = Serializer::readFloat(io, index);
                float speed = Serializer::readFloat(io, index);
                bool stopPoint = Serializer::readBool(io, index);
                bool endOfTraj = Serializer::readBool(io, index);
                Position p((float)x, (float)y, angle);
                TrajectoryPoint trajPoint(p, curvature, speed, stopPoint, endOfTraj);
                Server.printf(SPY_ORDER, "EditTrajPoint %u ", trajIndex);
                Server.println(SPY_ORDER, trajPoint);
                ret = motionControlSystem.updateTrajectory(trajIndex, trajPoint);

                if (ret != TRAJECTORY_EDITION_SUCCESS)
                {
                    motionControlSystem.stop_and_clear_trajectory();
                    break;
                }
                trajIndex++;
            }
        }
        else
        {
            Server.printf_err("EditTraj: wrong number of arguments\n");
        }
        io.clear();
        Serializer::writeEnum(ret, io);
    }
};


class DeleteTrajPts : public OrderImmediate, public Singleton<DeleteTrajPts>
{
public:
    DeleteTrajPts() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        uint8_t ret = TRAJECTORY_EDITION_FAILURE;
        if (io.size() == 4)
        {
            size_t index = 0;
            size_t trajIndex = Serializer::readUInt(io, index);
            ret = motionControlSystem.deleteTrajectoryPoints(trajIndex);
        }
        else
        {
            Server.printf_err("DeleteTrajPts: wrong number of arguments\n");
        }
        io.clear();
        Serializer::writeEnum(ret, io);
    }
};


class SetScore : public OrderImmediate, public Singleton<SetScore>
{
public:
    SetScore() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 4)
        {
            size_t index = 0;
            int32_t score = Serializer::readInt(io, index);
            dashboard.setScore(score);
            Server.printf(SPY_ORDER, "Score=%d\n", score);
            io.clear();
        }
        else
        {
            Server.printf_err("SetScore: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetNightLights : public OrderImmediate, public Singleton<SetNightLights>
{
public:
    SetNightLights() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 1)
        {
            size_t index = 0;
            uint8_t lightLevel = Serializer::readEnum(io, index);
            contextualLightning.setNightLight((ContextualLightning::NightLight)lightLevel);
            Server.printf(SPY_ORDER, "NightLightLevel=%u\n", lightLevel);
            io.clear();
        }
        else
        {
            Server.printf_err("SetNightLights: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetWarnings : public OrderImmediate, public Singleton<SetWarnings>
{
public:
    SetWarnings() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 1)
        {
            size_t index = 0;
            bool enable = Serializer::readBool(io, index);
            contextualLightning.enableWarnings(enable);
            Server.printf(SPY_ORDER, "Warnings=%d\n", enable);
            io.clear();
        }
        else
        {
            Server.printf_err("SetWarnings: wrong number of arguments\n");
            io.clear();
        }
    }
};

class ActuatorStop : public OrderImmediate, public Singleton<ActuatorStop>
{
public:
    ActuatorStop() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            actuatorMgr.stop();
            Server.printf(SPY_ORDER, "ActuatorStop\n");
            io.clear();
        }
        else
        {
            Server.printf_err("ActuatorStop: wrong number of arguments\n");
            io.clear();
        }
    }
};

class ActuatorGetPosition : public OrderImmediate, public Singleton<ActuatorGetPosition>
{
public:
    ActuatorGetPosition() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            io.clear();
            ActuatorPosition p = actuatorMgr.getPosition();
            Serializer::writeFloat(p.y, io);
            Serializer::writeFloat(p.z, io);
            Serializer::writeFloat(p.theta, io);
        }
        else
        {
            Server.printf_err("ActuatorGetPosition: wrong number of arguments\n");
            io.clear();
        }
    }
};

class EnableParkingBreak : public OrderImmediate, public Singleton<EnableParkingBreak>
{
public:
    EnableParkingBreak() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 1)
        {
            size_t index = 0;
            bool enable = Serializer::readBool(io, index);
            motionControlSystem.enableParkingBreak(enable);
            Server.printf(SPY_ORDER, "ParkingBreak=%d\n", enable);
            io.clear();
        }
        else
        {
            Server.printf_err("EnableParkingBreak: wrong number of arguments\n");
            io.clear();
        }
    }
};

class EnableHighSpeed : public OrderImmediate, public Singleton<EnableHighSpeed>
{
public:
    EnableHighSpeed() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 1)
        {
            size_t index = 0;
            bool enable = Serializer::readBool(io, index);
            motionControlSystem.enableHighSpeed(enable);
            Server.printf(SPY_ORDER, "EnableHighSpeed=%d\n", enable);
            io.clear();
        }
        else
        {
            Server.printf_err("EnableHighSpeed: wrong number of arguments\n");
            io.clear();
        }
    }
};

class DisplayColor : public OrderImmediate, public Singleton<DisplayColor>
{
public:
    DisplayColor() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 1)
        {
            size_t index = 0;
            Color color = (Color)Serializer::readEnum(io, index);
            if (color == VIOLET) {
                contextualLightning.setSideDisplay(ContextualLightning::SIDE_DISPLAY_VIOLET);
            }
            else if (color == JAUNE) {
                contextualLightning.setSideDisplay(ContextualLightning::SIDE_DISPLAY_ORANGE);
            }
            io.clear();
        }
        else
        {
            Server.printf_err("DisplayColor: wrong number of arguments\n");
            io.clear();
        }
    }
};



/********************
    DEBUG COMMANDS
*********************/


class Display : public OrderImmediate, public Singleton<Display>
{
public:
    Display() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            Server.print(motionControlSystem.getTunings());
            io.clear();
        }
        else
        {
            Server.printf_err("Display: wrong number of arguments\n");
            io.clear();
        }
    }
};


class Save : public OrderImmediate, public Singleton<Save>
{
public:
    Save() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            // TODO
            io.clear();
        }
        else
        {
            Server.printf_err("Save: wrong number of arguments\n");
            io.clear();
        }
    }
};


class LoadDefaults : public OrderImmediate, public Singleton<LoadDefaults>
{
public:
    LoadDefaults() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            // TODO
            io.clear();
        }
        else
        {
            Server.printf_err("LoadDefaults: wrong number of arguments\n");
            io.clear();
        }
    }
};


class GetPosition : public OrderImmediate, public Singleton<GetPosition>
{
public:
    GetPosition() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            io.clear();
            Position p = motionControlSystem.getPosition();
            Serializer::writeInt((int32_t)p.x, io);
            Serializer::writeInt((int32_t)p.y, io);
            Serializer::writeFloat(p.orientation, io);
        }
        else
        {
            Server.printf_err("GetPosition: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetControlLevel : public OrderImmediate, public Singleton<SetControlLevel>
{
public:
    SetControlLevel() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 1)
        {
            size_t index = 0;
            uint8_t controlLevel = Serializer::readEnum(io, index);
            motionControlSystem.setMotionControlLevel(controlLevel);
            Server.printf(SPY_ORDER, "MotionControlLevel=%u\n", motionControlSystem.getMotionControlLevel());
            io.clear();
        }
        else
        {
            Server.printf_err("SetControlLevel: wrong number of arguments\n");
            io.clear();
        }
    }
};


class StartManualMove : public OrderImmediate, public Singleton<StartManualMove>
{
public:
    StartManualMove() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            motionControlSystem.startManualMove();
            Server.printf(SPY_ORDER, "StartManualMove\n");
            io.clear();
        }
        else
        {
            Server.printf_err("StartManualMove: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetMaxSpeed : public OrderImmediate, public Singleton<SetMaxSpeed>
{
public:
    SetMaxSpeed() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 4)
        {
            size_t index = 0;
            float speed = Serializer::readFloat(io, index);
            motionControlSystem.setMaxSpeed(speed);
            Server.printf(SPY_ORDER, "SetMaxSpeed: %gmm/s\n", speed);
            io.clear();
        }
        else
        {
            Server.printf_err("SetMaxSpeed: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetAimDistance : public OrderImmediate, public Singleton<SetAimDistance>
{
public:
    SetAimDistance() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 4)
        {
            size_t index = 0;
            float aimDistance = Serializer::readFloat(io, index);
            motionControlSystem.setDistanceToDrive(aimDistance);
            Server.printf(SPY_ORDER, "SetAimDistance: %gmm\n", aimDistance);
            io.clear();
        }
        else
        {
            Server.printf_err("SetAimDistance: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetCurvature : public OrderImmediate, public Singleton<SetCurvature>
{
public:
    SetCurvature() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == sizeof(float))
        {
            size_t readIndex = 0;
            float curvature = Serializer::readFloat(io, readIndex);
            motionControlSystem.setCurvature(curvature);
            noInterrupts();
            directionController.setAimCurvature(curvature);
            interrupts();
            Server.printf(SPY_ORDER, "SetCurvature: %gm^-1\n", curvature);
            io.clear();
        }
        else
        {
            Server.printf_err("SetCurvature: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetDirAngle : public OrderImmediate, public Singleton<SetDirAngle>
{
public:
    SetDirAngle() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 4)
        {
            size_t readIndex = 0;
            int32_t angle = Serializer::readInt(io, readIndex);
            directionController.setMotorAngle(angle);
            Server.printf(SPY_ORDER, "SetDirAngle: %ddeg\n", angle);
            io.clear();
        }
        else
        {
            Server.printf_err("SetDirAngle: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetTranslationTunings : public OrderImmediate, public Singleton<SetTranslationTunings>
{
public:
    SetTranslationTunings() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 12)
        {
            size_t index = 0;
            float kp = Serializer::readFloat(io, index);
            float kd = Serializer::readFloat(io, index);
            float minAimSpeed = Serializer::readFloat(io, index);
            MotionControlTunings tunings = motionControlSystem.getTunings();
            tunings.translationKp = kp;
            tunings.translationKd = kd;
            tunings.minAimSpeed = minAimSpeed;
            motionControlSystem.setTunings(tunings);
            Server.printf(SPY_ORDER, "Translation Kp=%g Kd=%g MinSpeed=%g\n", kp, kd, minAimSpeed);
            io.clear();
        }
        else
        {
            Server.printf_err("SetTranslationTunings: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetTrajectoryTunings : public OrderImmediate, public Singleton<SetTrajectoryTunings>
{
public:
    SetTrajectoryTunings() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 12)
        {
            size_t index = 0;
            float k1 = Serializer::readFloat(io, index);
            float k2 = Serializer::readFloat(io, index);
            float dtt = Serializer::readFloat(io, index);
            MotionControlTunings tunings = motionControlSystem.getTunings();
            tunings.curvatureK1 = k1;
            tunings.curvatureK2 = k2;
            tunings.distanceMaxToTraj = dtt;
            motionControlSystem.setTunings(tunings);
            Server.printf(SPY_ORDER, "Trajectory K1=%g K2=%g Dist=%g\n", k1, k2, dtt);
            io.clear();
        }
        else
        {
            Server.printf_err("SetTrajectoryTunings: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetStoppingTunings : public OrderImmediate, public Singleton<SetStoppingTunings>
{
public:
    SetStoppingTunings() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 8)
        {
            size_t index = 0;
            float epsilon = Serializer::readFloat(io, index);
            uint32_t responseTime = Serializer::readUInt(io, index);
            MotionControlTunings tunings = motionControlSystem.getTunings();
            tunings.stoppedSpeed = epsilon;
            tunings.stoppingResponseTime = responseTime;
            motionControlSystem.setTunings(tunings);
            Server.printf(SPY_ORDER, "Stopping epsilon=%gmm/s delay=%ums\n", epsilon, responseTime);
            io.clear();
        }
        else
        {
            Server.printf_err("SetStoppingTunings: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetMaxAcceleration : public OrderImmediate, public Singleton<SetMaxAcceleration>
{
public:
    SetMaxAcceleration() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 4)
        {
            size_t index = 0;
            float acceleration = Serializer::readFloat(io, index);
            MotionControlTunings tunings = motionControlSystem.getTunings();
            tunings.maxAcceleration = acceleration;
            motionControlSystem.setTunings(tunings);
            Server.printf(SPY_ORDER, "MaxAcceleration=%gmm*s^-2\n", acceleration);
            io.clear();
        }
        else
        {
            Server.printf_err("SetMaxAcceleration: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetMaxDeceleration : public OrderImmediate, public Singleton<SetMaxDeceleration>
{
public:
    SetMaxDeceleration() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 4)
        {
            size_t index = 0;
            float deceleration = Serializer::readFloat(io, index);
            MotionControlTunings tunings = motionControlSystem.getTunings();
            tunings.maxDeceleration = deceleration;
            motionControlSystem.setTunings(tunings);
            Server.printf(SPY_ORDER, "MaxDeceleration=%gmm*s^-2\n", deceleration);
            io.clear();
        }
        else
        {
            Server.printf_err("SetMaxDeceleration: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetMaxCurvature : public OrderImmediate, public Singleton<SetMaxCurvature>
{
public:
    SetMaxCurvature() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 4)
        {
            size_t index = 0;
            float curvature = Serializer::readFloat(io, index);
            MotionControlTunings tunings = motionControlSystem.getTunings();
            tunings.maxCurvature = curvature;
            motionControlSystem.setTunings(tunings);
            Server.printf(SPY_ORDER, "MaxCurvature=%gm^-1\n", curvature);
            io.clear();
        }
        else
        {
            Server.printf_err("SetMaxCurvature: wrong number of arguments\n");
            io.clear();
        }
    }
};


class SetSmoke : public OrderImmediate, public Singleton<SetSmoke>
{
public:
    SetSmoke() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 1)
        {
            size_t index = 0;
            bool enable = Serializer::readBool(io, index);
            smokeMgr.enableSmoke(enable);
            Server.printf(SPY_ORDER, "SetSmoke=%d\n", enable);
            io.clear();
        }
        else
        {
            Server.printf_err("SetSmoke: wrong number of arguments\n");
            io.clear();
        }
    }
};


class GetSensorsLastUpdate : public OrderImmediate, public Singleton<GetSensorsLastUpdate>
{
public:
    GetSensorsLastUpdate() {}
    virtual void execute(std::vector<uint8_t> & io)
    {
        if (io.size() == 0)
        {
            io.clear();
            for (size_t i = 0; i < NB_SENSORS; i++) {
                Serializer::writeUInt(sensorMgr.getLastUpdateTime(i), io);
            }
        }
        else
        {
            Server.printf_err("GetSensorsLastUpdate: wrong number of arguments\n");
            io.clear();
        }
    }
};


#endif
