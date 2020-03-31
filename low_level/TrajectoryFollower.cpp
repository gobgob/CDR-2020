#include "TrajectoryFollower.h"
#include <Arduino.h>
#include "Utils.h"
#include "Serial.h"

/* Ecart maximal entre la consigne en courbure et la courbure réelle admissible
 * au démarrage. Unité : m^-1 */
#define CURVATURE_TOLERANCE 0.3

/* Durée maximale le la phase "MOVE_INIT" d'une trajectoire. Unité : ms */
#define TIMEOUT_MOVE_INIT 2000

#define INFINITE_DISTANCE INT32_MAX

/* mm/s (vitesse max en mode asservissement sur place) */
#define PARKING_MAX_SPEED 500

TrajectoryFollower::TrajectoryFollower(const float freqAsserv,
    volatile Position& position, volatile MoveStatus& moveStatus) :
    freqAsserv(freqAsserv),
    directionController(DirectionController::Instance()),
    motor(freqAsserv, PIN_ENC_MOT_A, PIN_ENC_MOT_B, PIN_MOT_INA, PIN_MOT_INB,
        PIN_MOT_PWM, PIN_MOT_SEL, PIN_MOT_CS, ANALOG_WRITE_RES, ANALOG_READ_RES),
    moveStatus(moveStatus),
    position(position),
    odometry(freqAsserv, position, currentTranslation, currentMovingSpeed),
    translationPID(currentTranslation, movingSpeedSetPoint,
        translationSetPoint, freqAsserv),
    endOfMoveMgr(currentMovingSpeed),
    curvaturePID(position, curvatureOrder, trajectoryPoint)
{
    movePhase = MOVE_ENDED;
    finalise_stop();
    moveInitTimer = 0;
    setMotionControlLevel(4);
    curvatureOrder = 0;
    currentMovingSpeed = 0;
    enableParkingBreak(false);
    updateTunings();
}

void TrajectoryFollower::setDistanceToDrive(float distance)
{
    noInterrupts();
    setDistanceToDriveFromInterrupt(distance);
    interrupts();
}

int TrajectoryFollower::getMovingDirection() const
{
    noInterrupts();
    MovePhase mp = movePhase;
    bool forward = isMovingForward();
    interrupts();
    if (mp == MOVE_ENDED) {
        return 0;
    }
    else if (forward) {
        return 1;
    }
    else {
        return -1;
    }
}

void TrajectoryFollower::setMaxSpeed(float maxSpeed)
{
    if (!trajectoryControlled) {
        noInterrupts();
        maxMovingSpeed = maxSpeed;
        interrupts();
    }
    else {
        Server.printf_err("TrajectoryFollower::setMaxSpeed : "
            "trajectory is controlled\n");
    }
}

void TrajectoryFollower::setCurvature(float curvature)
{
    noInterrupts();
    bool allowed = !trajectoryControlled || movePhase == MOVE_ENDED;
    interrupts();

    if (allowed) {
        noInterrupts();
        curvatureOrder = curvature;
        interrupts();
    }
    else {
        Server.printf_err("TrajectoryFollower::setCurvature : "
            "trajectory is controlled and move is started\n");
    }
}

float TrajectoryFollower::getCurvature() const
{
    float ret;
    noInterrupts();
    ret = curvatureOrder;
    interrupts();
    return ret;
}

float TrajectoryFollower::getCurrentMovingSpeed() const
{
    float ret;
    noInterrupts();
    ret = currentMovingSpeed;
    interrupts();
    return ret;
}

void TrajectoryFollower::setMotionControlLevel(uint8_t level)
{
    if (level > 4) {
        Server.printf_err("TrajectoryFollower::setMotionControlLevel : "
            "level > 4\n");
        return;
    }
    noInterrupts();
    translationControlled = level > 2;
    trajectoryControlled = level > 3;
    interrupts();
    motor.enableSpeedControl(level > 1);
}

uint8_t TrajectoryFollower::getMotionControlLevel() const
{
    uint8_t level = 1;
    level += (uint8_t)motor.isSpeedControlled();
    noInterrupts();
    level += (uint8_t)translationControlled;
    level += (uint8_t)trajectoryControlled;
    interrupts();
    return level;
}

bool TrajectoryFollower::isTrajectoryControlled() const
{
    bool ret;
    noInterrupts();
    ret = trajectoryControlled;
    interrupts();
    return ret;
}

const MotionControlTunings& TrajectoryFollower::tunings() const
{
    return motionControlTunings;
}

MotionControlTunings& TrajectoryFollower::tunings()
{
    return motionControlTunings;
}

void TrajectoryFollower::updateTunings()
{
    noInterrupts();
    maxAcceleration = motionControlTunings.maxAcceleration;
    maxDeceleration = motionControlTunings.maxDeceleration;
    minAimSpeed = motionControlTunings.minAimSpeed;
    stoppedSpeed = motionControlTunings.stoppedSpeed;
    translationPID.setTunings(motionControlTunings.translationKp, 0,
        motionControlTunings.translationKd);
    endOfMoveMgr.setTunings(motionControlTunings.stoppedSpeed,
        motionControlTunings.stoppingResponseTime);
    curvaturePID.setCurvatureLimits(-motionControlTunings.maxCurvature,
        motionControlTunings.maxCurvature);
    curvaturePID.setTunings(motionControlTunings.curvatureK1,
        motionControlTunings.curvatureK2);
    distanceMaxToTraj = motionControlTunings.distanceMaxToTraj;
    interrupts();
    motor.setTunings(motionControlTunings.speedKp,
        motionControlTunings.speedKi, motionControlTunings.speedKd);
}

void TrajectoryFollower::enableParkingBreak(bool enable)
{
    noInterrupts();
    if (enable) {
        parkingMaxMovingSpeed = PARKING_MAX_SPEED;
    }
    else {
        parkingMaxMovingSpeed = 0;
    }

    if (movePhase == MOVE_ENDED) {
        finalise_stop();
    }
    interrupts();
}

bool TrajectoryFollower::parkingBreakEnabled() const
{
    bool ret;
    noInterrupts();
    ret = parkingMaxMovingSpeed != 0;
    interrupts();
    return ret;
}

void TrajectoryFollower::getRawTicks(int32_t& leftTicks,
    int32_t& rightTicks) const
{
    odometry.getRawTicks(leftTicks, rightTicks);
}

const MotorEncoder& TrajectoryFollower::motorEncoder() const
{
    return motor;
}

MotorEncoder& TrajectoryFollower::motorEncoder()
{
    return motor;
}

void TrajectoryFollower::control()
{
    odometry.compute(isMovingForward());
    manageStop();
    checkPosition();

    if (movePhase == MOVE_INIT) {
        if (millis() - moveInitTimer > TIMEOUT_MOVE_INIT) {
            movePhase = MOVE_ENDED;
            moveStatus |= EXT_BLOCKED;
            finalise_stop();
        }
        else if (trajectoryControlled) {
            directionController.setAimCurvature(trajectoryPoint.getCurvature());

            if (ABS(directionController.getRealCurvature() -
                    trajectoryPoint.getCurvature()) < CURVATURE_TOLERANCE) {
                movePhase = MOVING;
                endOfMoveMgr.moveIsStarting();
                updateTranslationSetPoint();
            }
        }
        else {
            movePhase = MOVING;
            endOfMoveMgr.moveIsStarting();
            updateTranslationSetPoint();
        }
        movingSpeedSetPoint = 0;
    }
    else if (movePhase == MOVING) {
        if (trajectoryControlled) {
            /* MAJ curvatureOrder */
            curvaturePID.compute(isMovingForward());
            directionController.setAimCurvature(curvatureOrder);
        }

        if (translationControlled) {
            /* MAJ movingSpeedSetPoint */
            translationPID.compute();
        }
        else {
            movingSpeedSetPoint = ABS(maxMovingSpeed);
        }

        /* Limitation de l'accélération et de la décélération */
        if ((movingSpeedSetPoint - previousMovingSpeedSetpoint) *
            freqAsserv > maxAcceleration)
        {
            movingSpeedSetPoint = previousMovingSpeedSetpoint +
                maxAcceleration / freqAsserv;
        }
        else if ((previousMovingSpeedSetpoint - movingSpeedSetPoint) *
            freqAsserv > maxDeceleration)
        {
            movingSpeedSetPoint = previousMovingSpeedSetpoint -
                maxDeceleration / freqAsserv;
        }

        previousMovingSpeedSetpoint = movingSpeedSetPoint;
    }
    else if (movePhase == MOVE_ENDED) {
        /* Frein de parking actif */
        if (translationControlled) {
            /* MAJ movingSpeedSetPoint */
            translationPID.compute();
        }
        else {
            movingSpeedSetPoint = 0;
        }
    }
    else {
        movingSpeedSetPoint = 0;
    }

    if (translationControlled) {
        enforceSpeedLimits();
        if (isMovingForward()) {
            motor.setAimSpeedFromInterrupt(movingSpeedSetPoint);
        }
        else {
            motor.setAimSpeedFromInterrupt(-movingSpeedSetPoint);
        }
    }
    motor.compute();
}

void TrajectoryFollower::setTrajectoryPoint(TrajectoryPoint const& trajPoint)
{
    if (trajectoryControlled) {
        trajectoryPoint = trajPoint;
        maxMovingSpeed = trajectoryPoint.getAlgebricMaxSpeed();
    }
    else {
        Server.printf_err("TrajectoryFollower::setTrajectoryPoint : "
            "trajectory not controlled\n");
    }
}

void TrajectoryFollower::setDistanceToDriveFromInterrupt(float distance)
{
    if (translationControlled) {
        translationSetPointBuffer = currentTranslation + distance;
        if (movePhase == MOVING) {
            updateTranslationSetPoint();
        }
    }
    else {
        Server.printf_err("TrajectoryFollower::setDistanceToDriveFromInterrupt"
            " : translation not controlled\n");
    }
}

void TrajectoryFollower::setInfiniteDistanceToDrive()
{
    if (translationControlled) {
        translationSetPointBuffer = INFINITE_DISTANCE;
        if (movePhase == MOVING) {
            updateTranslationSetPoint();
        }
    }
    else {
        Server.printf_err("TrajectoryFollower::setInfiniteDistanceToDrive : "
            "translation not controlled\n");
    }
}

void TrajectoryFollower::startMove()
{
    if (movePhase == MOVE_ENDED) {
        movePhase = MOVE_INIT;
        moveInitTimer = millis();
    }
    else {
        Server.printf_err("TrajectoryFollower::startMove : "
            "move already started\n");
    }
}

MovePhase TrajectoryFollower::getMovePhase() const
{
    return movePhase;
}

bool TrajectoryFollower::isMovingForward() const
{
    return maxMovingSpeed >= 0;
}

void TrajectoryFollower::emergencyStop()
{
    if (movePhase != MOVE_ENDED) {
        if (translationControlled) {
            movePhase = BREAKING;
            moveStatus |= EMERGENCY_BREAK;
        }
        else {
            movePhase = MOVE_ENDED;
            finalise_stop();
        }
    }
    else {
        finalise_stop();
    }
}

void TrajectoryFollower::sendLogs()
{
    static MoveStatus lastMoveStatus = MOVE_OK;
    noInterrupts();
    MoveStatus ms = moveStatus;
    interrupts();

    Server.print(PID_TRANS, translationPID);
    Server.print(PID_TRAJECTORY, curvaturePID);
    Server.print(STOPPING_MGR, endOfMoveMgr);
    motor.sendLogs();

    if (ms != lastMoveStatus) {
        if (ms != MOVE_OK) {
            Server.printf_err("Move error: %u\n", (uint8_t)ms);
        }
        else {
            Server.printf("Move status is OK\n");
        }
        lastMoveStatus = ms;
    }
}

void TrajectoryFollower::finalise_stop()
{
    currentTranslation = 0;
    translationSetPoint = 0;
    translationSetPointBuffer = 0;
    movingSpeedSetPoint = 0;
    previousMovingSpeedSetpoint = 0;
    maxMovingSpeed = parkingMaxMovingSpeed;
    motor.setAimSpeedFromInterrupt(0);
    translationPID.resetIntegralError();
    translationPID.resetDerivativeError();
}

void TrajectoryFollower::manageStop()
{
    endOfMoveMgr.compute();
    if (translationControlled) {
        if (endOfMoveMgr.isStopped()) {
            if (movePhase == MOVING) {
                movePhase = MOVE_ENDED;
                if (trajectoryControlled && !trajectoryPoint.isStopPoint()) {
                    moveStatus |= EXT_BLOCKED;
                }
                finalise_stop();
            }
            else if (movePhase == BREAKING) {
                movePhase = MOVE_ENDED;
                finalise_stop();
            }
        }
    }
}

void TrajectoryFollower::checkPosition()
{
    if (movePhase == MOVING && trajectoryControlled) {
        if (position.distanceTo(trajectoryPoint.getPosition()) >
                distanceMaxToTraj) {
            movePhase = BREAKING;
            moveStatus |= FAR_AWAY;
        }
    }
}

void TrajectoryFollower::updateTranslationSetPoint()
{
    bool resetNeeded = (translationSetPoint < INFINITE_DISTANCE &&
        translationSetPointBuffer >= INFINITE_DISTANCE) ||
        (translationSetPoint >= INFINITE_DISTANCE &&
            translationSetPointBuffer < INFINITE_DISTANCE);

    translationSetPoint = translationSetPointBuffer;
    if (resetNeeded) {
        translationPID.resetIntegralError();
        translationPID.resetDerivativeError();
    }
}

void TrajectoryFollower::enforceSpeedLimits()
{
    /* Limitation de la vitesse */
    if (movingSpeedSetPoint > ABS(maxMovingSpeed)) {
        movingSpeedSetPoint = ABS(maxMovingSpeed);
    }
    else if (movingSpeedSetPoint < -ABS(maxMovingSpeed)) {
        movingSpeedSetPoint = -ABS(maxMovingSpeed);
    }

    if (ABS(movingSpeedSetPoint) < stoppedSpeed) {
        movingSpeedSetPoint = 0;
    }
    else if (ABS(movingSpeedSetPoint) < minAimSpeed) {
        if (movingSpeedSetPoint > 0) {
            movingSpeedSetPoint = minAimSpeed;
        }
        else {
            movingSpeedSetPoint = -minAimSpeed;
        }
    }
}
