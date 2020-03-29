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

    /* Lit les valeurs des encodeurs, et met � jour la position et les
     * vitesses en cons�quences. */
    void compute(bool movingForward);

    /* Renvoie directement les valeurs en ticks donn�es par les encodeurs */
    void getRawTicks(int32_t& leftTicks, int32_t& rightTicks) const;

private:
    /* Fr�quence d'appel de la m�thode 'compute'. Utilis�e pour le calcul des
     * vitesses. */
    const float freqAsserv;

    Encoder leftOdometryEncoder;
    Encoder rightOdometryEncoder;

    /* Position courante dans le r�f�rentiel de la table.
     * Unit�s mm;mm;radians */
    volatile Position& position;

    /* Distance parcourue par le robot en translation (avant-arri�re).
     * Unit� : mm */
    volatile float& currentTranslation;

    /* Vitesse moyenn�e du robot selon son axe avant-arri�re.
     * Unit� : mm/s */
    volatile float& translationSpeed;

    /* Variables utilis�es par `compute`, allou�es ici plut�t que sur la stack.
     */
    volatile int32_t deltaLeftOdometryTicks;
    volatile int32_t deltaRightOdometryTicks;
    volatile float half_deltaRotation_rad;
    volatile float currentAngle;
    volatile float corrector;
    volatile float deltaTranslation;

    Average<float, AVERAGE_SPEED_SIZE> averageTranslationSpeed;
};
