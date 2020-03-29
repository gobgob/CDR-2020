#include "CurvaturePID.h"
#include <Arduino.h>
#include "Utils.h"

CurvaturePID::CurvaturePID(volatile Position const& position,
    volatile float& curvatureOrder, TrajectoryPoint const& trajectoryPoint) :
    currentPosition(position),
    trajectoryPoint(trajectoryPoint),
    curvatureOrder(curvatureOrder)
{
    setCurvatureLimits(-20, 20);
    setTunings(0, 0);
    posError = 0;
    orientationError = 0;
    curvatureCorrection = 0;
}

void CurvaturePID::compute(bool movingForward)
{
    /* Asservissement sur trajectoire */

    Position posConsigne = trajectoryPoint.getPosition();
    float trajectoryCurvature = trajectoryPoint.getCurvature();
    posError = -(currentPosition.x - posConsigne.x) *
        sinf(posConsigne.orientation) + (currentPosition.y - posConsigne.y) *
        cosf(posConsigne.orientation);
    orientationError = fmodulo(
        currentPosition.orientation - posConsigne.orientation, TWO_PI);

    if (orientationError > PI) {
        orientationError -= TWO_PI;
    }

    if (movingForward) {
        curvatureCorrection = -k1 * posError - k2 * orientationError;
    }
    else {
        curvatureCorrection = -k1 * posError + k2 * orientationError;
    }
    curvatureOrder = constrain(trajectoryCurvature + curvatureCorrection,
        outMin, outMax);
}

void CurvaturePID::setCurvatureLimits(float min, float max)
{
    if (min >= max) { return; }

    outMin = min;
    outMax = max;
    curvatureOrder = constrain(curvatureOrder, outMin, outMax);
}

void CurvaturePID::setTunings(float k1, float k2)
{
    if (k1 < 0 || k2 < 0) {
        this->k1 = 0;
        this->k2 = 0;
    }
    this->k1 = k1;
    this->k2 = k2;
}

size_t CurvaturePID::printTo(Print& p) const
{
    return p.printf("%u_%g_%g_%g", millis(), posError * k1,
        orientationError * k2, curvatureCorrection);
}
