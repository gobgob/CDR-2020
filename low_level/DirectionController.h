#ifndef _DIRECTIONCONTROLLER_h
#define _DIRECTIONCONTROLLER_h

#include <Arduino.h>
#include <Printable.h>
#include <Dynamixel.h>
#include <DynamixelInterface.h>
#include <DynamixelMotor.h>
#include "SerialAX12.h"
#include "Singleton.h"
#include "Utils.h"
#include "Config.h"
#include "CommunicationServer.h"


/* Periode d'actualisation d'une requête AX12 */
#define CONTROL_PERIOD	10000 // µs

/* Angles limites, en degrés (uint16_t) */
#define DIR_ANGLE_MIN	    70   // doit être positif
#define DIR_ANGLE_ORIGIN    147
#define DIR_ANGLE_MAX	    200
#define DIR_TABLE_SIZE      301 // taille du tableau de conversion angle-courbure


enum DirectionControllerStatus
{
    DIRECTION_CONTROLLER_OK,
    DIRECTION_CONTROLLER_MOTOR_BLOCKED
};


class DirectionController : public Singleton<DirectionController>, public Printable
{
public:
	DirectionController() :
        directionMotor(SerialAX12, ID_AX12_DIRECTION)
	{
		aimCurvature = 0;
		updateAimAngle();
        realMotorAngle = DIR_ANGLE_ORIGIN;
		updateRealCurvature();
	}

    int init()
    {
        if (directionMotor.init() != DYN_STATUS_OK) {
            return EXIT_FAILURE;
        }
        directionMotor.enableTorque();
        directionMotor.jointMode();
        return EXIT_SUCCESS;
    }

    DirectionControllerStatus control()
	{
		static uint32_t lastUpdateTime = 0;
		static bool read = true;
        DirectionControllerStatus ret = DIRECTION_CONTROLLER_OK;
		
		if (micros() - lastUpdateTime >= CONTROL_PERIOD)
		{
            DynamixelStatus dynamixelStatus = DYN_STATUS_OK;
			lastUpdateTime = micros();
            if (read)
            {
                uint16_t angle;
                dynamixelStatus = directionMotor.currentPositionDegree(angle);
                if (angle <= 300) {
                    realMotorAngle = constrain(angle, DIR_ANGLE_MIN, DIR_ANGLE_MAX);
                }
                updateRealCurvature();
            }
            else
			{
                updateAimAngle();
                dynamixelStatus = directionMotor.goalPositionDegree(aimMotorAngle);
			}

            if (dynamixelStatus != DYN_STATUS_OK)
            {
                Server.printf_err("DirectionController: errno %u on operation #%d\n", dynamixelStatus, read);
            }
            if (dynamixelStatus & (DYN_STATUS_OVERLOAD_ERROR | DYN_STATUS_OVERHEATING_ERROR))
            {
                ret = DIRECTION_CONTROLLER_MOTOR_BLOCKED;
            }
            read = !read;
            Server.print(DIRECTION, *this);
		}

        return ret;
	}

    void recover()
    {
        directionMotor.recoverTorque();
    }
	

    /*--- Méthodes à appeller depuis une interrupt ---*/
	void setAimCurvature(float curvature)
	{
		aimCurvature = curvature;
	}
	float getRealCurvature() const
	{
		return realCurvature;
	}
    /*--- fin ---*/

	uint16_t getMotorAngle() const
	{
		return realMotorAngle;
	}
	void setMotorAngle(uint16_t angle)
	{
        if (angle < DIR_ANGLE_MIN || angle > DIR_ANGLE_MAX) {
            return;
        }
		aimMotorAngle = angle;
        noInterrupts();
        aimCurvature = angle_curvature_table[aimMotorAngle];
        interrupts();
	}

	size_t printTo(Print& p) const
	{
		return p.printf("%u_%g_%g", millis(), aimCurvature, realCurvature);
	}

private:
	void updateRealCurvature()
	{
		noInterrupts();
		realCurvature = angle_curvature_table[realMotorAngle];
		interrupts();
	}

	void updateAimAngle()
	{
		noInterrupts();
		float aimCurvature_cpy = aimCurvature;
		interrupts();
        size_t min_index = DIR_ANGLE_MIN;
        size_t max_index = DIR_ANGLE_MAX;
        size_t index = (DIR_ANGLE_MIN + DIR_ANGLE_MAX) / 2;

        while (max_index - min_index > 1)
        {
            if (aimCurvature_cpy < angle_curvature_table[index])
            {
                min_index = index;
            }
            else if (aimCurvature_cpy > angle_curvature_table[index])
            {
                max_index = index;
            }
            else
            {
                break;
            }
            index = (min_index + max_index) / 2;
        }
        aimMotorAngle = index;
	}
	
	/* Courbure, en m^-1 */
	volatile float aimCurvature;
	volatile float realCurvature;

	/* Angles des AX12, en degrés */
	uint16_t aimMotorAngle;
	uint16_t realMotorAngle;

	/* L'AX12 de direction */
	DynamixelMotor directionMotor;

    /* Table de conversion Angle-Courbure */
    static float angle_curvature_table[DIR_TABLE_SIZE];
};


#endif
