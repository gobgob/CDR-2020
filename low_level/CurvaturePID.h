#pragma once

#include <Printable.h>
#include "TrajectoryPoint.h"
#include "Position.h"


class CurvaturePID : public Printable
{
public:
    CurvaturePID(volatile Position const& position,
        volatile float& curvatureOrder, TrajectoryPoint const& trajectoryPoint);

    void compute(bool movingForward);
    void setCurvatureLimits(float min, float max);
    void setTunings(float k1, float k2);

    float getPositionError() const { return posError; }
    float getOrientationError() const { return orientationError; }

    size_t printTo(Print& p) const;

private:
    /* Position courante du robot */
    volatile Position const & currentPosition;

    /* Point de trajectoire courant (consigne à suivre) */
    TrajectoryPoint const & trajectoryPoint;

    /* Courbure consigne. Unitée : m^-1 */
    volatile float & curvatureOrder;

    /* Correctif de courbure. Unité : m^-1 */
    volatile float curvatureCorrection;

    float k1, k2;
    float posError, orientationError;
    float outMax, outMin;
};
