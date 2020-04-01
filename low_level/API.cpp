#include "API.h"
#include "OrderMgr.h"

#include "Serial.h"
#include "Serializer.h"
#include "TrajectoryFollower.h"
#include "MoveState.h"
#include "Position.h"
#include "TrajectoryPoint.h"
#include "SensorsMgr.h"


/* ################################################## *
*  # Ici est définie la correspondance ID <-> Ordre # *
*  # (il faut ajouter le START_ID à l'index du      # *
*  # tableau pour avoir l'ID utilisé dans la trame) # *
*  ################################################## */

/* Ordres à réponse immédiate */
void OrderMgr::initImmediateOrderList()
{
    immediateOrderList[0x00] = &Ping::Instance();
    immediateOrderList[0x01] = &GetColor::Instance();
    immediateOrderList[0x02] = &EditPosition::Instance();
    immediateOrderList[0x03] = &SetPosition::Instance();
    immediateOrderList[0x04] = &AppendToTraj::Instance();
    immediateOrderList[0x05] = &EditTraj::Instance();
    immediateOrderList[0x06] = &DeleteTrajPts::Instance();
    immediateOrderList[0x07] = &SetScore::Instance();
    immediateOrderList[0x08] = &SetParkingBreak::Instance();

    immediateOrderList[0x10] = &Display::Instance();
    immediateOrderList[0x11] = &Save::Instance();
    immediateOrderList[0x12] = &LoadDefaults::Instance();
    immediateOrderList[0x13] = &GetPosition::Instance();
    immediateOrderList[0x14] = &SetControlLevel::Instance();
    immediateOrderList[0x15] = &StartManualMove::Instance();
    immediateOrderList[0x16] = &SetMaxSpeed::Instance();
    immediateOrderList[0x17] = &SetAimDistance::Instance();
    immediateOrderList[0x18] = &SetCurvature::Instance();
    immediateOrderList[0x19] = &SetDirAngle::Instance();
    immediateOrderList[0x1A] = &SetTranslationTunings::Instance();
    immediateOrderList[0x1B] = &SetTrajectoryTunings::Instance();
    immediateOrderList[0x1C] = &SetStoppingTunings::Instance();
    immediateOrderList[0x1D] = &SetMaxAcceleration::Instance();
    immediateOrderList[0x1E] = &SetMaxDeceleration::Instance();
    immediateOrderList[0x1F] = &SetMaxCurvature::Instance();
    immediateOrderList[0X20] = &SetSpeedTunings::Instance();
    immediateOrderList[0x21] = &SetAimSpeed::Instance();
    immediateOrderList[0x22] = &SetMotorPWM::Instance();
    immediateOrderList[0x23] = &GetRawTicks::Instance();
}

/* Ordres longs */
void OrderMgr::initLongOrderList()
{
    longOrderList[0x00] = &FollowTrajectory::Instance();
    longOrderList[0x01] = &Stop::Instance();
    longOrderList[0x02] = &WaitForJumper::Instance();
    longOrderList[0x03] = &StartChrono::Instance();
}


/* ################################################# *
*  # Implémentation des ordres à réponse immédiate # *
*  ################################################# */

ORDER_IMMEDIATE_EXECUTE(Ping)
{
    Serializer::writeInt(0, output);
}

ORDER_IMMEDIATE_EXECUTE(GetColor)
{
    // todo
    Serializer::writeEnum((uint8_t)0, output);
}

ORDER_IMMEDIATE_EXECUTE(EditPosition)
{
    size_t index = 0;
    int32_t x = Serializer::readInt(input, index);
    int32_t y = Serializer::readInt(input, index);
    float angle = Serializer::readFloat(input, index);
    Position p = motionControlSystem.getPosition();
    p.x += (float)x;
    p.y += (float)y;
    p.setOrientation(p.orientation + (float)angle);
    Server.printf(SPY_ORDER, "EditPosition, result=");
    Server.print(SPY_ORDER, p);
    motionControlSystem.setPosition(p);
}

ORDER_IMMEDIATE_EXECUTE(SetPosition)
{
    size_t index = 0;
    int32_t x = Serializer::readInt(input, index);
    int32_t y = Serializer::readInt(input, index);
    float angle = Serializer::readFloat(input, index);
    Position p((float)x, (float)y, angle);
    Server.printf(SPY_ORDER, "SetPosition");
    Server.print(SPY_ORDER, p);
    motionControlSystem.setPosition(p);
}

ORDER_IMMEDIATE_EXECUTE(AppendToTraj)
{
    uint8_t ret = TRAJECTORY_EDITION_FAILURE;

    if (input.size() % 22 == 0) {
        for (size_t i = 0; i < input.size(); i += 22) {
            size_t index = i;
            int32_t x = Serializer::readInt(input, index);
            int32_t y = Serializer::readInt(input, index);
            float angle = Serializer::readFloat(input, index);
            float curvature = Serializer::readFloat(input, index);
            float speed = Serializer::readFloat(input, index);
            bool stopPoint = Serializer::readBool(input, index);
            bool endOfTraj = Serializer::readBool(input, index);
            Position p((float)x, (float)y, angle);
            TrajectoryPoint trajPoint(p, curvature, speed, stopPoint, endOfTraj);

            Server.printf(SPY_ORDER, "AppendToTraj: ");
            Server.println(SPY_ORDER, trajPoint);

            ret = motionControlSystem.appendToTrajectory(trajPoint);
            if (ret != TRAJECTORY_EDITION_SUCCESS) {
                motionControlSystem.stopAndClearTrajectory();
                Server.printf_err("AppendToTraj: TRAJECTORY_EDITION_FAILURE");
                break;
            }
        }
    }
    else {
        Server.printf_err("AppendToTraj: wrong number of arguments\n");
    }

    Serializer::writeEnum(ret, output);
}

ORDER_IMMEDIATE_EXECUTE(EditTraj)
{
    uint8_t ret = TRAJECTORY_EDITION_FAILURE;

    if (input.size() > 0 && (input.size() - 1) % 22 == 0) {
        size_t index = 0;
        size_t trajIndex = Serializer::readUInt(input, index);

        for (size_t i = 1; i < input.size(); i += 22) {
            index = i;
            int32_t x = Serializer::readInt(input, index);
            int32_t y = Serializer::readInt(input, index);
            float angle = Serializer::readFloat(input, index);
            float curvature = Serializer::readFloat(input, index);
            float speed = Serializer::readFloat(input, index);
            bool stopPoint = Serializer::readBool(input, index);
            bool endOfTraj = Serializer::readBool(input, index);
            Position p((float)x, (float)y, angle);
            TrajectoryPoint trajPoint(p, curvature, speed, stopPoint, endOfTraj);

            Server.printf(SPY_ORDER, "EditTrajPoint %u ", trajIndex);
            Server.println(SPY_ORDER, trajPoint);

            ret = motionControlSystem.updateTrajectory(trajIndex, trajPoint);
            if (ret != TRAJECTORY_EDITION_SUCCESS) {
                motionControlSystem.stopAndClearTrajectory();
                Server.printf_err("EditTraj: TRAJECTORY_EDITION_FAILURE");
                break;
            }
            trajIndex++;
        }
    }
    else {
        Server.printf_err("EditTraj: wrong number of arguments\n");
    }

    Serializer::writeEnum(ret, output);
}

ORDER_IMMEDIATE_EXECUTE(DeleteTrajPts)
{
    size_t index = 0;
    size_t trajIndex = Serializer::readUInt(input, index);
    uint8_t ret = motionControlSystem.deleteTrajectoryPoints(trajIndex);
    Serializer::writeEnum(ret, output);
}

ORDER_IMMEDIATE_EXECUTE(SetScore)
{
    size_t index = 0;
    int32_t score = Serializer::readInt(input, index);
    // todo
    Server.printf(SPY_ORDER, "Score=%d\n", score);
}

ORDER_IMMEDIATE_EXECUTE(SetParkingBreak)
{
    size_t index = 0;
    bool enable = Serializer::readBool(input, index);
    motionControlSystem.trajFollower().enableParkingBreak(enable);
}

/* Debug API */
ORDER_IMMEDIATE_EXECUTE(Display)
{
    Server.print(motionControlSystem.trajFollower().tunings());
}

ORDER_IMMEDIATE_EXECUTE(Save)
{
    // todo
}

ORDER_IMMEDIATE_EXECUTE(LoadDefaults)
{
    // todo
}

ORDER_IMMEDIATE_EXECUTE(GetPosition)
{
    Position p = motionControlSystem.getPosition();
    Serializer::writeInt((int32_t)p.x, output);
    Serializer::writeInt((int32_t)p.y, output);
    Serializer::writeFloat(p.orientation, output);
}

ORDER_IMMEDIATE_EXECUTE(SetControlLevel)
{
    size_t index = 0;
    uint8_t controlLevel = Serializer::readEnum(input, index);
    motionControlSystem.trajFollower().setMotionControlLevel(controlLevel);
    Server.printf(SPY_ORDER, "MotionControlLevel=%u\n", controlLevel);
}

ORDER_IMMEDIATE_EXECUTE(StartManualMove)
{
    motionControlSystem.startManualMove();
    Server.printf(SPY_ORDER, "StartManualMove\n");
}

ORDER_IMMEDIATE_EXECUTE(SetMaxSpeed)
{
    size_t index = 0;
    float speed = Serializer::readFloat(input, index);
    motionControlSystem.trajFollower().setMaxSpeed(speed);
    Server.printf(SPY_ORDER, "SetMaxSpeed: %gmm/s\n", speed);
}

ORDER_IMMEDIATE_EXECUTE(SetAimDistance)
{
    size_t index = 0;
    float aimDistance = Serializer::readFloat(input, index);
    motionControlSystem.trajFollower().setDistanceToDrive(aimDistance);
    Server.printf(SPY_ORDER, "SetAimDistance: %gmm\n", aimDistance);
}

ORDER_IMMEDIATE_EXECUTE(SetCurvature)
{
    size_t index = 0;
    float curvature = Serializer::readFloat(input, index);
    motionControlSystem.trajFollower().setCurvature(curvature);
    noInterrupts();
    directionController.setAimCurvature(curvature);
    interrupts();
    Server.printf(SPY_ORDER, "SetCurvature: %gm^-1\n", curvature);
}

ORDER_IMMEDIATE_EXECUTE(SetDirAngle)
{
    size_t index = 0;
    int32_t angle = Serializer::readInt(input, index);
    directionController.setMotorAngle(angle);
    Server.printf(SPY_ORDER, "SetDirAngle: %ddeg\n", angle);
}

ORDER_IMMEDIATE_EXECUTE(SetTranslationTunings)
{
    size_t index = 0;
    float kp = Serializer::readFloat(input, index);
    float kd = Serializer::readFloat(input, index);
    float minAimSpeed = Serializer::readFloat(input, index);
    motionControlTunings.translationKp = kp;
    motionControlTunings.translationKd = kd;
    motionControlTunings.minAimSpeed = minAimSpeed;
    motionControlSystem.trajFollower().updateTunings();
    Server.printf(SPY_ORDER, "Translation Kp=%g Kd=%g MinSpeed=%g\n", kp, kd,
        minAimSpeed);
}

ORDER_IMMEDIATE_EXECUTE(SetTrajectoryTunings)
{
    size_t index = 0;
    float k1 = Serializer::readFloat(input, index);
    float k2 = Serializer::readFloat(input, index);
    float dtt = Serializer::readFloat(input, index);
    motionControlTunings.curvatureK1 = k1;
    motionControlTunings.curvatureK2 = k2;
    motionControlTunings.distanceMaxToTraj = dtt;
    motionControlSystem.trajFollower().updateTunings();
    Server.printf(SPY_ORDER, "Trajectory K1=%g K2=%g Dist=%g\n", k1, k2, dtt);
}

ORDER_IMMEDIATE_EXECUTE(SetStoppingTunings)
{
    size_t index = 0;
    float epsilon = Serializer::readFloat(input, index);
    uint32_t responseTime = Serializer::readUInt(input, index);
    motionControlTunings.stoppedSpeed = epsilon;
    motionControlTunings.stoppingResponseTime = responseTime;
    motionControlSystem.trajFollower().updateTunings();
    Server.printf(SPY_ORDER, "Stopping epsilon=%gmm/s delay=%ums\n", epsilon,
        responseTime);
}

ORDER_IMMEDIATE_EXECUTE(SetMaxAcceleration)
{
    size_t index = 0;
    float acceleration = Serializer::readFloat(input, index);
    motionControlTunings.maxAcceleration = acceleration;
    motionControlSystem.trajFollower().updateTunings();
    Server.printf(SPY_ORDER, "MaxAcceleration=%gmm*s^-2\n", acceleration);
}

ORDER_IMMEDIATE_EXECUTE(SetMaxDeceleration)
{
    size_t index = 0;
    float deceleration = Serializer::readFloat(input, index);
    motionControlTunings.maxDeceleration = deceleration;
    motionControlSystem.trajFollower().updateTunings();
    Server.printf(SPY_ORDER, "MaxDeceleration=%gmm*s^-2\n", deceleration);
}

ORDER_IMMEDIATE_EXECUTE(SetMaxCurvature)
{
    size_t index = 0;
    float curvature = Serializer::readFloat(input, index);
    motionControlTunings.maxCurvature = curvature;
    motionControlSystem.trajFollower().updateTunings();
    Server.printf(SPY_ORDER, "MaxCurvature=%gm^-1\n", curvature);
}

ORDER_IMMEDIATE_EXECUTE(SetSpeedTunings)
{
    size_t index = 0;
    float kp = Serializer::readFloat(input, index);
    float ki = Serializer::readFloat(input, index);
    float kd = Serializer::readFloat(input, index);
    motionControlTunings.speedKp = kp;
    motionControlTunings.speedKi = ki;
    motionControlTunings.speedKd = kd;
    motionControlSystem.trajFollower().updateTunings();
    Server.printf(SPY_ORDER, "Speed Kp=%g Ki=%g Kd=%g\n", kp, ki, kd);
}

ORDER_IMMEDIATE_EXECUTE(SetAimSpeed)
{
    size_t index = 0;
    float aimSpeed = Serializer::readFloat(input, index);
    motionControlSystem.trajFollower().motorEncoder().setAimSpeed(aimSpeed);
    Server.printf(SPY_ORDER, "SetAimSpeed: %gmm/s", aimSpeed);
}

ORDER_IMMEDIATE_EXECUTE(SetMotorPWM)
{
    size_t index = 0;
    float pwm = Serializer::readFloat(input, index);
    motionControlSystem.trajFollower().motorEncoder().setRawPWM(pwm);
    Server.printf(SPY_ORDER, "SetRawPWM: %g%%\n", pwm);
}

ORDER_IMMEDIATE_EXECUTE(GetRawTicks)
{
    int32_t left, right;
    motionControlSystem.trajFollower().getRawTicks(left, right);
    Serializer::writeInt(left, output);
    Serializer::writeInt(right, output);
}


/* ################################### *
*  # Implémentation des ordres longs # *
*  ################################### */

/* FollowTrajectory */
ORDER_LONG_LAUNCH(FollowTrajectory)
{
    Server.printf(SPY_ORDER, "FollowTrajectory\n");
    motionControlSystem.followTrajectory();
}

ORDER_LONG_EXECUTE(FollowTrajectory)
{
    if (!motionControlSystem.isMovingToDestination()) {
        status = motionControlSystem.getMoveStatus();
        finished = true;
    }
}

ORDER_LONG_TERMINATE(FollowTrajectory)
{
    Server.printf(SPY_ORDER, "End FollowTrajectory with status %u\n", status);
    Serializer::writeInt((int32_t)status, output);
}

/* Stop */
ORDER_LONG_LAUNCH(Stop)
{
    Server.printf(SPY_ORDER, "Stop");
    motionControlSystem.stopAndClearTrajectory();
}

ORDER_LONG_EXECUTE(Stop)
{
    if (!motionControlSystem.isMovingToDestination())
    {
        finished = true;
    }
}

ORDER_LONG_TERMINATE(Stop)
{}

/* WaitForJumper */
ORDER_LONG_LAUNCH(WaitForJumper)
{
    Server.printf(SPY_ORDER, "WaitForJumper\n");
    state = WAIT_FOR_INSERTION;
    debounceTimer = 0;
}

ORDER_LONG_EXECUTE(WaitForJumper)
{
    uint8_t jumperDetected = digitalRead(PIN_GET_JUMPER);
    switch (state)
    {
    case WaitForJumper::WAIT_FOR_INSERTION:
        if (jumperDetected)
        {
            state = WAIT_FOR_REMOVAL;
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

ORDER_LONG_TERMINATE(WaitForJumper)
{}

/* StartChrono */
ORDER_LONG_LAUNCH(StartChrono)
{
    Server.printf(SPY_ORDER, "StartChrono");
    chrono = millis();
}

ORDER_LONG_EXECUTE(StartChrono)
{
    if (millis() - chrono > 100000)
    {
        finished = true;
    }
}

ORDER_LONG_TERMINATE(StartChrono)
{
    motionControlSystem.stopAndClearTrajectory();
    //actuatorMgr.stop();
    //actuatorMgr.disableAll();
}
