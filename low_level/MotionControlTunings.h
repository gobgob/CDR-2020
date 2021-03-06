#ifndef _MOTION_CONTROL_TUNINGS_h
#define _MOTION_CONTROL_TUNINGS_h

#include "Printable.h"


class MotionControlTunings : public Printable
{
public:
	MotionControlTunings()
	{
		setDefault();
	}

	void setDefault()
	{
        maxAcceleration = 2000;
        maxDeceleration = 12000;
        maxCurvature = 4.5;
        minAimSpeed = 400;

        stoppedSpeed = 50;
        stoppingResponseTime = 200;

        curvatureK1 = 0.025;
        curvatureK2 = 7.0;
        distanceMaxToTraj = 50;

        translationKp = 8.0;
        translationKd = 0.5;
	}

    void setHighSpeed()
    {
        maxAcceleration = 2000;
        maxDeceleration = 12000;
        maxCurvature = 4.5;
        minAimSpeed = 400;

        stoppedSpeed = 50;
        stoppingResponseTime = 200;

        curvatureK1 = 0.0025;
        curvatureK2 = 0.7;
        distanceMaxToTraj = 300;

        translationKp = 6.0;
        translationKd = 0.9;
    }

    size_t printTo(Print& p) const
    {
        size_t ret = 0;
        ret += p.printf("maxAcceleration=%g\n", maxAcceleration);
        ret += p.printf("maxDeceleration=%g\n", maxDeceleration);
        ret += p.printf("maxCurvature=%g\n", maxCurvature);
        ret += p.printf("minAimSpeed=%g\n", minAimSpeed);
        ret += p.printf("stoppedSpeed=%g\n", stoppedSpeed);
        ret += p.printf("stoppingResponseTime=%u\n", stoppingResponseTime);
        ret += p.printf("curvatureK1=%g\n", curvatureK1);
        ret += p.printf("curvatureK2=%g\n", curvatureK2);
        ret += p.printf("distanceMaxToTraj=%g\n", distanceMaxToTraj);
        ret += p.printf("translationKp=%g\n", translationKp);
        ret += p.printf("translationKd=%g\n", translationKd);

        return ret;
    }

    float maxAcceleration;      // mm*s^-2
    float maxDeceleration;      // mm*s^-2
    float maxCurvature;         // m^-1
    float minAimSpeed;          // mm*s^-1

    float stoppedSpeed;             // Vitesse en dessous de laquelle on consid�re �tre � l'arr�t. mm/s
    uint32_t stoppingResponseTime;  // ms

    float curvatureK1;
    float curvatureK2;
    float distanceMaxToTraj;    // mm

    float translationKp;
    float translationKd;
};


#endif
