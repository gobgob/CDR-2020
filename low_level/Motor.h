#ifndef _MOTOR_h
#define _MOTOR_h

#include "Arduino.h"
#include "Config.h"

#define MOTOR_MAX_SPEED 1800.0  // mm/s
#define MOTOR_PWM_RES   16      // bits
#define MOTOR_PWM_FREQ  50      // Hz
#define MOTOR_MAX_PWM   6553
#define MOTOR_MIN_PWM   3277

/*
    20ms PPM period and 16-bit PWM
    Min speed (negative) : 1ms ppm pusle => pwm=3277
    Max speed (positive) : 2ms ppm pulse => pwm=6553
    Zero speed : 1.5ms ppm pulse => pwm=4915
*/


class Motor
{
public:
	Motor()
	{
  //      // Passage de la pin en OUTPUT
		//pinMode(PIN_VESC, OUTPUT);

		//// La résolution du PWM est 16bits (0 - 65535)
		//analogWriteResolution(16);

		//// Réglage de la fréquence des PWM (Hz)
		//analogWriteFrequency(PIN_VESC, 50);

		//// Initialisation : Moteurs arrêtés
  //      run(0);
	}

    /* Speed in mm/s */
    void run(float speed)
    {
        //static const float a = (float)(MOTOR_MAX_PWM - MOTOR_MIN_PWM) / (float)(2 * MOTOR_MAX_SPEED);
        //static const float b = (float)(MOTOR_MAX_PWM + MOTOR_MIN_PWM) / 2;
        //speed = constrain(speed, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
        //float pwm = a * speed + b;
        //analogWrite(PIN_VESC, (int)pwm);
    }
};


#endif
