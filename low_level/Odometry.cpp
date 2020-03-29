#include "Odometry.h"
#include <Arduino.h>
#include "Utils.h"

Odometry::Odometry(const float freqAsserv, volatile Position& p,
    volatile float& currentTranslation, volatile float& translationSpeed) :
    freqAsserv(freqAsserv),
    leftOdometryEncoder(PIN_B_LEFT_ENCODER, PIN_A_LEFT_ENCODER),
    rightOdometryEncoder(PIN_A_RIGHT_ENCODER, PIN_B_RIGHT_ENCODER),
    position(p),
    currentTranslation(currentTranslation),
    translationSpeed(translationSpeed)
{
    deltaLeftOdometryTicks = 0;
    deltaRightOdometryTicks = 0;
    half_deltaRotation_rad = 0;
    currentAngle = 0;
    corrector = 0;
    deltaTranslation = 0;
}

void Odometry::compute(bool movingForward)
{
    // Calcul du mouvement de chaque roue depuis le dernier asservissement
    deltaLeftOdometryTicks = leftOdometryEncoder.readAndReset();
    deltaRightOdometryTicks = rightOdometryEncoder.readAndReset();

    // Mise à jour de la position et de l'orientattion
    deltaTranslation = (((float)deltaLeftOdometryTicks +
        (float)deltaRightOdometryTicks) / 2) * TICK_TO_MM;
    half_deltaRotation_rad = (((float)deltaRightOdometryTicks -
        (float)deltaLeftOdometryTicks) / 4) * TICK_TO_RADIANS;
    currentAngle = position.orientation + half_deltaRotation_rad;
    position.setOrientation(position.orientation + half_deltaRotation_rad * 2);
    corrector = 1 - square(half_deltaRotation_rad) / 6;
    position.x += corrector * deltaTranslation * cosf(currentAngle);
    position.y += corrector * deltaTranslation * sinf(currentAngle);

    // Mise à jour de currentTranslation
    if (movingForward) {
        currentTranslation += deltaTranslation;
    }
    else {
        currentTranslation -= deltaTranslation;
    }

    // Mise à jour de la vitesse de translation
    averageTranslationSpeed.add(deltaTranslation * freqAsserv);
    translationSpeed = averageTranslationSpeed.value();
}

void Odometry::getRawTicks(int32_t& leftTicks, int32_t& rightTicks) const
{
    noInterrupts();
    leftTicks = deltaLeftOdometryTicks;
    rightTicks = deltaRightOdometryTicks;
    interrupts();
}
