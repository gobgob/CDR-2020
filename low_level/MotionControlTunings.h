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
        maxAcceleration = 1000;
        maxDeceleration = 12000;
        maxCurvature = 10;
        minAimSpeed = 80;

        stoppedSpeed = 10;
        stoppingResponseTime = 100;

        curvatureK1 = 0.1;
        curvatureK2 = 12;

        translationKp = 6;
        translationKd = 0.2;
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
        ret += p.printf("translationKp=%g\n", translationKp);
        ret += p.printf("translationKd=%g\n", translationKd);

        return ret;
    }

    float maxAcceleration;      // mm*s^-2
    float maxDeceleration;      // mm*s^-2
    float maxCurvature;         // m^-1
    float minAimSpeed;          // mm*s^-1

    float stoppedSpeed;             // Vitesse en dessous de laquelle on considère être à l'arrêt. mm/s
    uint32_t stoppingResponseTime;  // ms

    float curvatureK1;
    float curvatureK2;

    float translationKp;
    float translationKd;
};


#endif
