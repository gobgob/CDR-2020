/*
    Name:       low_level.ino
    Created:    02/03/2019
    Author:	    Sylvain Gaultier
*/

#include <Wire.h>
#include <Encoder.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"
#include "OrderMgr.h"
#include "MotionControlSystem.h"
#include "ActuatorMgr.h"
#include "SensorsMgr.h"
#include "Serializer.h"
#include "Serial.h"

#define ODOMETRY_REPORT_PERIOD  20  // ms


void setup() {}
void loop()
{
    OrderMgr orderManager;
    //MotionControlSystem &motionControlSystem = MotionControlSystem::Instance();
    DirectionController &directionController = DirectionController::Instance();
    SensorsMgr &sensorMgr = SensorsMgr::Instance();
    ActuatorMgr &actuatorMgr = ActuatorMgr::Instance();

    IntervalTimer motionControlTimer;
    //uint32_t odometryReportTimer = 0;
    std::vector<uint8_t> odometryReport;

    Wire.begin();
    init_serial_ports();

    if (directionController.init() != DIR_STATUS_OK) {
        // todo
    }
    if (actuatorMgr.init() != ACT_STATUS_OK) {
        // todo
    }
    if (sensorMgr.init() != EXIT_SUCCESS) {
        // todo
    }

    motionControlTimer.priority(253);
    motionControlTimer.begin(motionControlInterrupt, PERIOD_ASSERV);

    while (true) {
        //uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
        //t1 = micros();
        orderManager.execute();
        //t2 = micros();
        //if (directionController.control() != DIR_STATUS_NOT_UPDATED) {
        //    Server.print(DIRECTION, directionController);
        //}
        //t3 = micros();
        //actuatorMgr.control();
        //t6 = micros();
        //sensorMgr.update(motionControlSystem.getMovingDirection());
        //t7 = micros();

        //if (millis() - odometryReportTimer > ODOMETRY_REPORT_PERIOD) {
        //    odometryReportTimer = millis();
        //    //Serial.print(sensorMgr);

        //    odometryReport.clear();
        //    Position p = motionControlSystem.getPosition();
        //    Serializer::writeInt(p.x, odometryReport);
        //    Serializer::writeInt(p.y, odometryReport);
        //    Serializer::writeFloat(p.orientation, odometryReport);
        //    Serializer::writeFloat(motionControlSystem.getCurvature(), odometryReport);
        //    Serializer::writeUInt(motionControlSystem.getTrajectoryIndex(), odometryReport);
        //    Serializer::writeBool(motionControlSystem.isMovingForward(), odometryReport);
        //    sensorMgr.appendValuesToVect(odometryReport);
        //    Server.sendData(ODOMETRY_AND_SENSORS, odometryReport);

        //    motionControlSystem.sendLogs();
        //}

        //static uint32_t dbg = 0;
        //if (millis() - dbg > 200) {
        //    dbg = millis();
        //    int32_t leftTicks, rightTicks;
        //    motionControlSystem.getRawTicks(leftTicks, rightTicks);
        //    Position p = motionControlSystem.getPosition();
        //    Server.printf("g=%d\td=%d\n", leftTicks, rightTicks);
        //    Server.print(p);
        //}

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
