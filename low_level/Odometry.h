#pragma once

#include <Encoder.h>
#include <stdint.h>
#include "Position.h"
#include "Average.h"
#include "Config.h"

/*
    Classe permettant de calculer la vitesse et la position d'un ensemble
    d'encodeurs.
*/
class Odometry
{
public:
    Odometry(const float freqAsserv, volatile Position& p,
        volatile float& currentTranslation, volatile float& translationSpeed);

    /* Lit les valeurs des encodeurs, et met à jour la position et les
     * vitesses en conséquences. */
    void compute(bool movingForward);

    /* Renvoie directement les valeurs en ticks données par les encodeurs */
    void getRawTicks(int32_t& leftTicks, int32_t& rightTicks) const;

private:
    /* Fréquence d'appel de la méthode 'compute'. Utilisée pour le calcul des
     * vitesses. */
    const float freqAsserv;

    Encoder leftOdometryEncoder;
    Encoder rightOdometryEncoder;

    /* Position courante dans le référentiel de la table.
     * Unités mm;mm;radians */
    volatile Position& position;

    /* Distance parcourue par le robot en translation (avant-arrière).
     * Unité : mm */
    volatile float& currentTranslation;

    /* Vitesse moyennée du robot selon son axe avant-arrière.
     * Unité : mm/s */
    volatile float& translationSpeed;

    /* Variables utilisées par `compute`, allouées ici plutôt que sur la stack.
     */
    volatile int32_t deltaLeftOdometryTicks;
    volatile int32_t deltaRightOdometryTicks;
    volatile float half_deltaRotation_rad;
    volatile float currentAngle;
    volatile float corrector;
    volatile float deltaTranslation;

    Average<float, AVERAGE_SPEED_SIZE> averageTranslationSpeed;
};
