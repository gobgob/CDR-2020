#pragma once

#include "DirectionController.h"
#include "Odometry.h"
#include "PID.h"
#include "CurvaturePID.h"
#include "StoppingMgr.h"
#include "MotorEncoder.h"
#include "Position.h"
#include "MoveState.h"
#include "MotionControlTunings.h"
#include <stdint.h>

class TrajectoryFollower
{
public:
    void setDistanceToDrive(float distance);
    int getMovingDirection() const;
    void setMaxSpeed(float maxSpeed);
    void setCurvature(float curvature);
    float getCurvature() const;
    float getCurrentMovingSpeed() const;
    void setMotionControlLevel(uint8_t level);
    uint8_t getMotionControlLevel() const;
    bool isTrajectoryControlled() const;
    const MotionControlTunings& tunings() const;
    MotionControlTunings& tunings();
    void updateTunings();
    void enableParkingBreak(bool enable);
    bool parkingBreakEnabled() const;
    void getRawTicks(int32_t& leftTicks, int32_t& rightTicks) const;

    const MotorEncoder& motorEncoder() const;
    MotorEncoder& motorEncoder();

    void sendLogs();

protected:
    TrajectoryFollower(const float freqAsserv, volatile Position& position,
        volatile MoveStatus& moveStatus);

    void control();
    void setTrajectoryPoint(TrajectoryPoint const& trajPoint);
    void setDistanceToDriveFromInterrupt(float distance);
    void setInfiniteDistanceToDrive();
    void startMove();
    MovePhase getMovePhase() const;
    bool isMovingForward() const;
    void emergencyStop();

private:
    void finalise_stop();
    void manageStop();
    void checkPosition();
    void updateTranslationSetPoint();
    void enforceSpeedLimits();

    /* Fréquence d'asservissement (Hz) */
    const float freqAsserv;

    DirectionController& directionController;
    MotorEncoderWithInterruptAccess motor;
    volatile MoveStatus& moveStatus;
    volatile MovePhase movePhase;

    /* Position courante */
    volatile Position& position;

    /* Point de trajectoire courant pour asservissement */
    TrajectoryPoint trajectoryPoint;

    /* Calcul de la position et des vitesses */
    Odometry odometry;

    /* Asservissement en translation */
    PID translationPID;
    /* Consigne (mm) */
    volatile float translationSetPoint;
    /* Sauvegarde de la consigne avant son application effective */
    volatile float translationSetPointBuffer;
    /* Position réelle sur la courbe de trajectoire (mm) */
    volatile float currentTranslation;
    /* Vitesse consigne (mm/s) */
    volatile float movingSpeedSetPoint;

    /* Gestion de l'arrêt */
    StoppingMgr endOfMoveMgr;
    /* Vitesse de translation réelle (mm/s) */
    volatile float currentMovingSpeed;

    /* Asservissement sur trajectoire */
    CurvaturePID curvaturePID;
    /* Courbure consigne (m^-1) */
    volatile float curvatureOrder;

    /* Activation des différents PID */
    bool trajectoryControlled;  // Asservissement sur trajectoire
    bool translationControlled; // Asservissement en translation

    /* Vitesse (algébrique) de translation maximale : une vitesse négative
     * correspond à une marche arrière. Unité : mm/s */
    float maxMovingSpeed;

    /* Accélérations maximale (variation maximale de movingSpeedSetpoint).
     * Unité : mm*s^-2 */
    float maxAcceleration;
    float maxDeceleration;

    /* Vitesse non nulle minimale pouvant être donnée en tant que consigne.
     * Unité : mm/s */
    float minAimSpeed;

    /* En dessous de cette vitesse, on considère être à l'arrêt.
     * Unité : mm/s */
    float stoppedSpeed;

    /* Distance (entre notre position et la trajectoire) au delà de laquelle on
     * abandonne la trajectoire. Unité : mm */
    float distanceMaxToTraj;

    /* Pour le calcul de l'accélération. Unité : mm/s */
    float previousMovingSpeedSetpoint;

    /* Pour le timeout de la phase MOVE_INIT */
    uint32_t moveInitTimer;

    /* Pour le réglage des paramètres des PID, BlockingMgr et StoppingMgr */
    MotionControlTunings motionControlTunings;

    /* Vitesse maximale en mode asservissement sur place (vaut 0 si la
     * fonctionalité est désactivée) */
    volatile float parkingMaxMovingSpeed;
};

class TrajectoryFollowerWithInterruptAccess : public TrajectoryFollower
{
public:
    TrajectoryFollowerWithInterruptAccess(const float freqAsserv,
        volatile Position& position, volatile MoveStatus& moveStatus) :
        TrajectoryFollower(freqAsserv, position, moveStatus)
    {}

    using TrajectoryFollower::control;
    using TrajectoryFollower::setTrajectoryPoint;
    using TrajectoryFollower::setDistanceToDriveFromInterrupt;
    using TrajectoryFollower::setInfiniteDistanceToDrive;
    using TrajectoryFollower::startMove;
    using TrajectoryFollower::getMovePhase;
    using TrajectoryFollower::isMovingForward;
    using TrajectoryFollower::emergencyStop;
};
