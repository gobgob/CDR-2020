#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define DIR_INFINITE_CURVATURE	(1e6f)

/* Distance entre l'essieu avant et arrière */
#define WHEELBASE_LENGTH        135.0f  // mm

/* Angles limites, en degrés (uint16_t) */
#define DIR_ANGLE_MIN	    105 // doit être positif
#define DIR_ANGLE_ORIGIN    150
#define DIR_ANGLE_MAX	    240

float angleToCurvature(uint16_t angle)
{
    int a_deg = (int)angle - DIR_ANGLE_ORIGIN;
    float a_rad = (float)a_deg * M_PI / 180.0f;

    if (a_deg > 0) {
        if (a_deg < 90) {
            return 1.0f / (tanf(a_rad - M_PI_2) * WHEELBASE_LENGTH / 1000.0f);
        }
        else {
            return -DIR_INFINITE_CURVATURE;
        }
    }
    else if (a_deg < 0) {
        if (a_deg > -90) {
            return 1.0f / (tanf(a_rad + M_PI_2) * WHEELBASE_LENGTH / 1000.0f);
        }
        else {
            return DIR_INFINITE_CURVATURE;
        }
    }
    else {
        return 0.0;
    }
}

uint16_t curvatureToAngle(float curvature)
{
    if (curvature > 0) {
        if (curvature < DIR_INFINITE_CURVATURE) {
            float angle = (atanf(1.0f / (curvature * WHEELBASE_LENGTH / 
                1000.0f)) - M_PI_2) * 180.0f / M_PI;
            return round(angle) + DIR_ANGLE_ORIGIN;
        }
        else {
            return DIR_ANGLE_ORIGIN - 90;
        }
    }
    else if (curvature < 0) {
        if (curvature > -DIR_INFINITE_CURVATURE) {
            float angle = (atanf(1.0f / (curvature * WHEELBASE_LENGTH /
                1000.0f)) + M_PI_2) * 180.0f / M_PI;
            return round(angle) + DIR_ANGLE_ORIGIN;
        }
        else {
            return DIR_ANGLE_ORIGIN + 90;
        }
    }
    else  {
        return DIR_ANGLE_ORIGIN;
    }
}

int main ()
{
	printf("Hello World\n");
	FILE* pFile;
	pFile = fopen("result.txt", "w");
	
	for (int i = 0; i <= 300; i++) {
		fprintf(pFile, "%u, %u\n", i, curvatureToAngle(angleToCurvature(i)));
	}

	fclose(pFile);
}
