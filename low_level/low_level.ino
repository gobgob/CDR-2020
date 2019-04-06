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
#include <Adafruit_NeoPixel.h>
#include "Config.h"
#include "OrderMgr.h"
#include "MotionControlSystem.h"
#include "ContextualLightning.h"
#include "ActuatorMgr.h"
#include "SensorsMgr.h"
#include "Serializer.h"
#include "Dashboard.h"
#include "SerialAX12.h"

#define ODOMETRY_REPORT_PERIOD  20  // ms


void setup() {}
void loop()
{
    OrderMgr orderManager;
    MotionControlSystem &motionControlSystem = MotionControlSystem::Instance();
    DirectionController &directionController = DirectionController::Instance();
    SensorsMgr &sensorMgr = SensorsMgr::Instance();
    ActuatorMgr &actuatorMgr = ActuatorMgr::Instance();
    Dashboard &dashboard = Dashboard::Instance();
    ContextualLightning &contextualLightning = ContextualLightning::Instance();
    IntervalTimer motionControlTimer;
    IntervalTimer actuatorMgrTimer;
    uint32_t odometryReportTimer = 0;
    std::vector<uint8_t> odometryReport;

    Wire.begin();
    dashboard.init();
    if (Server.begin() != 0)
    {
        dashboard.setErrorLevel(Dashboard::WEAK_ERROR);
    }
    SerialAX12.begin(SERIAL_AX12_BAUDRATE, SERIAL_AX12_TIMEOUT);
    if (directionController.init() != EXIT_SUCCESS)
    {
        dashboard.setErrorLevel(Dashboard::WEAK_ERROR);
    }
    if (actuatorMgr.init() != EXIT_SUCCESS)
    {
        dashboard.setErrorLevel(Dashboard::STRONG_WARNING);
    }
    if (sensorMgr.init() != EXIT_SUCCESS)
    {
        dashboard.setErrorLevel(Dashboard::STRONG_WARNING);
    }

    motionControlTimer.priority(253);
    motionControlTimer.begin(motionControlInterrupt, PERIOD_ASSERV);
    actuatorMgrTimer.priority(252);
    actuatorMgrTimer.begin(actuatorMgrInterrupt, ACT_MGR_INTERRUPT_PERIOD);

    contextualLightning.setNightLight(ContextualLightning::NIGHT_LIGHT_LOW);

    while (true)
    {
        //uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
        //t1 = micros();
        orderManager.execute();
        //t2 = micros();
        directionController.control();
        //t3 = micros();
        actuatorMgr.mainLoopControl();
        //t4 = micros();
        dashboard.update();
        //t5 = micros();
        contextualLightning.update();
        //t6 = micros();
        sensorMgr.update(motionControlSystem.getMovingDirection());
        //t7 = micros();

        if (millis() - odometryReportTimer > ODOMETRY_REPORT_PERIOD)
        {
            odometryReportTimer = millis();
            //Serial.println(sensorMgr);

            odometryReport.clear();
            Position p = motionControlSystem.getPosition();
            Serializer::writeInt(p.x, odometryReport);
            Serializer::writeInt(p.y, odometryReport);
            Serializer::writeFloat(p.orientation, odometryReport);
            Serializer::writeFloat(motionControlSystem.getCurvature(), odometryReport);
            Serializer::writeUInt(motionControlSystem.getTrajectoryIndex(), odometryReport);
            Serializer::writeBool(motionControlSystem.isMovingForward(), odometryReport);
            sensorMgr.appendValuesToVect(odometryReport);
            actuatorMgr.appendSensorsValuesToVect(odometryReport);
            Server.sendData(ODOMETRY_AND_SENSORS, odometryReport);

            motionControlSystem.sendLogs();
        }
        //t8 = micros();

        //if (t8 - t1 > 3000) {
        //    Serial.print(t8 - t1); 
        //    Serial.print(" [");
        //    Serial.print(t2 - t1);
        //    Serial.print(" ; ");
        //    Serial.print(t3 - t2);
        //    Serial.print(" ; ");
        //    Serial.print(t4 - t3);
        //    Serial.print(" ; ");
        //    Serial.print(t5 - t4);
        //    Serial.print(" ; ");
        //    Serial.print(t6 - t5);
        //    Serial.print(" ; ");
        //    Serial.print(t7 - t6);
        //    Serial.print(" ; ");
        //    Serial.print(t8 - t7);
        //    Serial.println("]");
        //}
        //uint32_t dt = t8 - t7;
        //if (dt > 100) {
        //    Serial.println(dt);
        //}
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
