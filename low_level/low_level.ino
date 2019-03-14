/*
    Name:       low_level.ino
    Created:    02/03/2019
    Author:	    Sylvain Gaultier
*/

#include <Ethernet.h>
#include <Wire.h>
#include <Dynamixel.h>
#include <DynamixelInterface.h>
#include <DynamixelMotor.h>
#include <Encoder.h>
#include <ToF_sensor.h>
#include <A4988.h>
#include <Adafruit_LEDBackpack.h>
#include "Config.h"
#include "OrderMgr.h"
#include "MotionControlSystem.h"
#include "ContextualLightning.h"
#include "ActuatorMgr.h"
#include "SensorsMgr.h"
#include "Serializer.h"

#define ODOMETRY_REPORT_PERIOD  20  // ms


void setup()
{
    pinMode(PIN_DEL_WARNING, OUTPUT);
    pinMode(PIN_DEL_ERROR, OUTPUT);
    digitalWrite(PIN_DEL_WARNING, HIGH);

    if (Server.begin() != 0)
    {
        digitalWrite(PIN_DEL_ERROR, HIGH);
        delay(500);
    }
}


void loop()
{
    OrderMgr orderManager;
    MotionControlSystem &motionControlSystem = MotionControlSystem::Instance();
    DirectionController &directionController = DirectionController::Instance();
    SensorsMgr &sensorMgr = SensorsMgr::Instance();
    ActuatorMgr &actuatorMgr = ActuatorMgr::Instance();

    IntervalTimer motionControlTimer;
    motionControlTimer.priority(253);
    motionControlTimer.begin(motionControlInterrupt, PERIOD_ASSERV);

    uint32_t odometryReportTimer = 0;
    std::vector<uint8_t> odometryReport;

    uint32_t delTimer = 0;
    bool delState = true;

    while (true)
    {
        orderManager.execute();
        directionController.control();
        sensorMgr.update();
        actuatorMgr.mainLoopControl();

        if (millis() - odometryReportTimer > ODOMETRY_REPORT_PERIOD)
        {
            odometryReportTimer = millis();

            odometryReport.clear();
            Position p = motionControlSystem.getPosition();
            Serializer::writeInt(p.x, odometryReport);
            Serializer::writeInt(p.y, odometryReport);
            Serializer::writeFloat(p.orientation, odometryReport);
            Serializer::writeFloat(motionControlSystem.getCurvature(), odometryReport);
            Serializer::writeUInt(motionControlSystem.getTrajectoryIndex(), odometryReport);
            Serializer::writeBool(motionControlSystem.isMovingForward(), odometryReport);
            sensorMgr.appendValuesToVect(odometryReport);
            Server.sendData(ODOMETRY_AND_SENSORS, odometryReport);

            motionControlSystem.sendLogs();
        }

        if (millis() - delTimer > 500)
        {
            delState = !delState;
            digitalWrite(PIN_DEL_WARNING, delState);
            delTimer = millis();
        }
    }
}


void motionControlInterrupt()
{
    static MotionControlSystem &motionControlSystem = MotionControlSystem::Instance();
    motionControlSystem.control();
}


void actuatorMgrInterrupt()
{
    static ActuatorMgr &actuatorMgr = ActuatorMgr::Instance();
    actuatorMgr.interruptControl();
}


/* Ce bout de code permet de compiler avec std::vector */
namespace std {
    void __throw_bad_alloc()
    {
        while (true)
        {
            Server.printf_err("Unable to allocate memory\n");
            delay(500);
        }
    }

    void __throw_length_error(char const*e)
    {
        while (true)
        {
            Server.printf_err(e);
            Server.println_err();
            delay(500);
        }
    }

    void __throw_out_of_range(char const*e)
    {
        while (true)
        {
            Server.printf_err(e);
            Server.println_err();
            delay(500);
        }
    }

    void __throw_out_of_range_fmt(char const*e, ...)
    {
        while (true)
        {
            Server.printf_err(e);
            Server.println_err();
            delay(500);
        }
    }
}
