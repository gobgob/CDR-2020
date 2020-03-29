#include "MotionControlSystem.h"
#include <Arduino.h>
#include "Utils.h"
#include "Serial.h"

/* Distance entre deux points d'une trajectoire (mm) */
#define TRAJECTORY_STEP 20

MotionControlSystem::MotionControlSystem() :
    trajectoryFollower(FREQ_ASSERV, position, moveStatus)
{
    travellingToDestination = false;
    trajectoryIndex = 0;
    trajectoryComplete = false;
    moveStatus = MOVE_OK;
}

void MotionControlSystem::control()
{
    static bool wasTravellingToDestination = false;

    trajectoryFollower.control();
    if (!travellingToDestination) {
        return;
    }

    MovePhase movePhase = trajectoryFollower.getMovePhase();

    if (currentTrajectory.size() <= trajectoryIndex) {
        if (movePhase != BREAKING) {
            stopAndClearTrajectoryFromInterrupt();
            if (movePhase != MOVE_ENDED || !wasTravellingToDestination) {
                moveStatus |= EMPTY_TRAJ;
            }
            travellingToDestination = false;
            wasTravellingToDestination = false;
        }
        return;
    }

    if (!wasTravellingToDestination) {
        /* Démarrage du suivi de trajectoire */
        trajectoryFollower.setTrajectoryPoint(
            currentTrajectory.at(trajectoryIndex));
        updateDistanceToTravelFromInterrupt();
        trajectoryFollower.startMove();
        wasTravellingToDestination = true;
    }
    else if (movePhase == MOVING && !currentTrajectory.at(trajectoryIndex).isStopPoint())
    {
        const Position& trajPoint =
            currentTrajectory.at(trajectoryIndex).getPosition();
        float sign;
        if (trajectoryFollower.isMovingForward()) {
            sign = 1;
        }
        else {
            sign = -1;
        }

        if (((position.x - trajPoint.x) * cosf(trajPoint.orientation) +
            (position.y - trajPoint.y) * sinf(trajPoint.orientation)) *
            sign > 0)
        {
            if (currentTrajectory.size() > trajectoryIndex + 1) {
                trajectoryIndex++;
                updateDistanceToTravelFromInterrupt();
                trajectoryFollower.setTrajectoryPoint(
                    currentTrajectory.at(trajectoryIndex));
            }
            else {
                moveStatus |= EMPTY_TRAJ;
                stopAndClearTrajectoryFromInterrupt();
            }
        }
    }
    else if (movePhase == MOVE_ENDED) {
        if (currentTrajectory.at(trajectoryIndex).isEndOfTrajectory() ||
            moveStatus != MOVE_OK)
        {
            stopAndClearTrajectoryFromInterrupt();
            travellingToDestination = false;
            wasTravellingToDestination = false;
        }
        else {
            if (currentTrajectory.size() > trajectoryIndex + 1) {
                trajectoryIndex++;
                updateDistanceToTravelFromInterrupt();
                trajectoryFollower.setTrajectoryPoint(
                    currentTrajectory.at(trajectoryIndex));
                trajectoryFollower.startMove();
            }
            else {
                moveStatus |= EMPTY_TRAJ;
                stopAndClearTrajectoryFromInterrupt();
                travellingToDestination = false;
                wasTravellingToDestination = false;
            }
        }
    }
}

void MotionControlSystem::stopAndClearTrajectoryFromInterrupt()
{
    trajectoryFollower.emergencyStop();
    trajectoryIndex = 0;
    currentTrajectory.clear();
    trajectoryComplete = false;
}

void MotionControlSystem::updateDistanceToTravelFromInterrupt()
{
    for (size_t i = trajectoryIndex; i < currentTrajectory.size(); i++) {
        if (currentTrajectory.at(i).isStopPoint()) {
            float distanceToDrive = ((float)i - (float)trajectoryIndex + 1) *
                TRAJECTORY_STEP;
            trajectoryFollower.setDistanceToDriveFromInterrupt(distanceToDrive);
            return;
        }
    }
    trajectoryFollower.setInfiniteDistanceToDrive();
}

void MotionControlSystem::followTrajectory()
{
    if (trajectoryFollower.isTrajectoryControlled()) {
        noInterrupts();
        moveStatus = MOVE_OK;
        travellingToDestination = true;
        interrupts();
    }
    else {
        Server.printf_err("MotionControlSystem::followTrajectory : "
            "trajectory is not controlled\n");
    }
}

void MotionControlSystem::startManualMove()
{
    if (!trajectoryFollower.isTrajectoryControlled()) {
        noInterrupts();
        moveStatus = MOVE_OK;
        trajectoryFollower.startMove();
        interrupts();
        Server.printf("Manual move started");
    }
    else {
        Server.printf_err("MotionControlSystem::startManualMove : "
            "trajectory is controlled\n");
    }
}

void MotionControlSystem::stopAndClearTrajectory()
{
    uint32_t t = millis();
    Position p = getPosition();
    noInterrupts();
    stopAndClearTrajectoryFromInterrupt();
    interrupts();
    Server.printf("Emergency stop started at: %u\n", t);
    Server.printf("pos= ");
    Server.println(p);
}

bool MotionControlSystem::isMovingToDestination() const
{
    noInterrupts();
    bool moving = travellingToDestination;
    interrupts();
    return moving;
}

uint8_t MotionControlSystem::appendToTrajectory(TrajectoryPoint trajectoryPoint)
{
    if (!trajectoryComplete) {
        noInterrupts();
        currentTrajectory.push_back(trajectoryPoint);
        interrupts();
        if (trajectoryPoint.isEndOfTrajectory()) {
            trajectoryComplete = true;
        }
        const Position& p = trajectoryPoint.getPosition();
        Server.printf(AIM_TRAJECTORY, "%u_%g_%g", millis(), p.x, p.y);
        return TRAJECTORY_EDITION_SUCCESS;
    }
    else {
        return TRAJECTORY_EDITION_FAILURE;
    }
}

uint8_t MotionControlSystem::updateTrajectory(size_t index,
    TrajectoryPoint trajectoryPoint)
{
    if (index >= currentTrajectory.size() || index < trajectoryIndex) {
        return TRAJECTORY_EDITION_FAILURE;
    }
    if (index == trajectoryIndex && travellingToDestination) {
        return TRAJECTORY_EDITION_FAILURE;
    }

    noInterrupts();
    if (currentTrajectory.at(index).isEndOfTrajectory()) {
        trajectoryComplete = false;
    }
    currentTrajectory.at(index) = trajectoryPoint;
    if (trajectoryPoint.isEndOfTrajectory()) {
        trajectoryComplete = true;
        if (currentTrajectory.size() > index + 1) {
            currentTrajectory.erase(currentTrajectory.begin() + index + 1,
                currentTrajectory.end());
        }
    }
    interrupts();

    return TRAJECTORY_EDITION_SUCCESS;
}

uint8_t MotionControlSystem::deleteTrajectoryPoints(size_t index)
{
    if (index >= currentTrajectory.size() || index < trajectoryIndex) {
        return TRAJECTORY_EDITION_FAILURE;
    }
    if (index == trajectoryIndex && travellingToDestination) {
        return TRAJECTORY_EDITION_FAILURE;
    }

    noInterrupts();
    trajectoryComplete = false;
    currentTrajectory.erase(currentTrajectory.begin() + index,
        currentTrajectory.end());
    interrupts();

    return TRAJECTORY_EDITION_SUCCESS;
}

Position MotionControlSystem::getPosition() const
{
    static Position p;
    noInterrupts();
    p = position;
    interrupts();
    return p;
}

void MotionControlSystem::setPosition(Position p)
{
    noInterrupts();
    position = p;
    interrupts();
}

MoveStatus MotionControlSystem::getMoveStatus() const
{
    static MoveStatus ms;
    noInterrupts();
    ms = moveStatus;
    interrupts();
    return ms;
}

size_t MotionControlSystem::getTrajectoryIndex() const
{
    noInterrupts();
    size_t ret = trajectoryIndex;
    interrupts();
    return ret;
}

const TrajectoryFollower& MotionControlSystem::trajFollower() const
{
    return trajectoryFollower;
}

TrajectoryFollower& MotionControlSystem::trajFollower()
{
    return trajectoryFollower;
}

void MotionControlSystem::sendLogs()
{
    trajectoryFollower.sendLogs();
}
