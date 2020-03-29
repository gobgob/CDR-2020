#pragma once

#include "Singleton.h"
#include "TrajectoryFollower.h"
#include "MoveState.h"
#include "Position.h"
#include "TrajectoryPoint.h"
#include "MotionControlTunings.h"
#include <vector>
#include <stdint.h>

/* Fréquence d'asservissement (Hz) */
#define FREQ_ASSERV 1000

/* Période d'asservissement (µs) */
#define PERIOD_ASSERV (1000000 / FREQ_ASSERV)

/* Codes de retour */
#define TRAJECTORY_EDITION_SUCCESS 0
#define TRAJECTORY_EDITION_FAILURE 1

class MotionControlSystem : public Singleton<MotionControlSystem>
{
public:
    /* Constructeur */
    MotionControlSystem();

    /* Méthode à appeller durant l'interruption d'asservissement */
    void control();

private:
    void stopAndClearTrajectoryFromInterrupt();
    void updateDistanceToTravelFromInterrupt();

 public:
    void followTrajectory();
    void startManualMove();
    void stopAndClearTrajectory();
    bool isMovingToDestination() const;
    uint8_t appendToTrajectory(TrajectoryPoint trajectoryPoint);
    uint8_t updateTrajectory(size_t index, TrajectoryPoint trajectoryPoint);
    uint8_t deleteTrajectoryPoints(size_t index);

    Position getPosition() const;
    void setPosition(Position p);
    MoveStatus getMoveStatus() const;
    size_t getTrajectoryIndex() const;

    const TrajectoryFollower& trajFollower() const;
    TrajectoryFollower& trajFollower();

    void sendLogs();

private:
    TrajectoryFollowerWithInterruptAccess trajectoryFollower;
    volatile Position position;
    volatile MoveStatus moveStatus;

    /* Indique si le robot est en train de parcourir la trajectoire courante */
    volatile bool travellingToDestination;

    /* Point courant de la trajectoire courante */
    volatile size_t trajectoryIndex;

    std::vector<TrajectoryPoint> currentTrajectory;
    bool trajectoryComplete;
};
