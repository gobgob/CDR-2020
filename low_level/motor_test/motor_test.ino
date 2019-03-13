/*
    Name:       motor_test.ino
    Created:	02/03/2019 22:13:41
    Author:     sg-msi\Sylvain
*/

#include <Arduino.h>
#include "Motor.h"

Motor motor;
char cmd[100] = "";
size_t i = 0;

void setup()
{
}

void loop()
{
    if (Serial.available())
    {
        char c = Serial.read();
        if (c == '\n')
        {
            cmd[i] = '\0';
            float speed = atof(cmd);
            Serial.print("Speed= ");
            Serial.println(speed);
            motor.run(speed);
            i = 0;
        }
        else if (i < 100)
        {
            cmd[i] = c;
            i++;
        }
        else
        {
            i = 0;
        }
    }
}
