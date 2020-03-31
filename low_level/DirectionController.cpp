#include "DirectionController.h"
#include <Arduino.h>
#include "Config.h"
#include "Serial.h"

/* Periode d'actualisation d'une requête AX12 */
#define DIR_CONTROL_PERIOD  10000 // µs

/* Angles limites, en degrés (uint16_t) */
#define DIR_ANGLE_MIN       105 // doit être positif
#define DIR_ANGLE_ORIGIN    150
#define DIR_ANGLE_MAX       240

/* Délai entre le blocage de l'AX12 et la tentative de récupération */
#define RECOVER_DELAY   5000    // ms

DirectionController::DirectionController() :
    directionMotor(SerialAX12, ID_AX12_DIRECTION)
{
    aimCurvature = 0;
    updateAimAngle();
    realMotorAngle = DIR_ANGLE_ORIGIN;
    updateRealCurvature();
    blocked = true;
    blockedTimer = 0;
    is_init = false;
}

DirectionControllerStatus DirectionController::init()
{
    OneWireStatus ret;

    ret = directionMotor.init();
    if (ret != OW_STATUS_OK) {
        Server.printf_err("DirectionController: com error at 'init'\n");
        return DIR_STATUS_FAILURE;
    }

    ret = directionMotor.enableTorque();
    if (ret != OW_STATUS_OK) {
        Server.printf_err("DirectionController: com error at 'enableTorque'\n");
        return DIR_STATUS_FAILURE;
    }

    ret = directionMotor.jointMode();
    if (ret != OW_STATUS_OK) {
        Server.printf_err("DirectionController: com error at 'jointMode'\n");
        return DIR_STATUS_FAILURE;
    }

    setBlocked(directionMotor.environmentError());
    
    if (directionMotor.error()) {
        Server.printf_err("DirectionController: AX12 status %u\n",
            (uint8_t)directionMotor.status());
    }

    is_init = true;
    return DIR_STATUS_OK;
}

DirectionControllerStatus DirectionController::control()
{
    static uint32_t lastUpdateTime = 0;
    static bool read = true;
    uint32_t now = micros();

    if (!is_init || now - lastUpdateTime < DIR_CONTROL_PERIOD) {
        return DIR_STATUS_NOT_UPDATED;
    }

    lastUpdateTime = now;
    DirectionControllerStatus ret = DIR_STATUS_OK;

    if (blocked) {
        if (directionMotor.ping() != OW_STATUS_OK) {
            ret = DIR_STATUS_FAILURE;
        }

        if (!directionMotor.environmentError()) {
            setBlocked(false);
        }
        else if (directionMotor.recoverableError() && millis() - blockedTimer > RECOVER_DELAY) {
            if (directionMotor.recoverTorque() == OW_STATUS_OK) {
                setBlocked(false);
            }
        }
    }
    else {
        if (read) {
            uint16_t angle;
            if (directionMotor.currentPositionDegree(angle) != OW_STATUS_OK) {
                ret = DIR_STATUS_FAILURE;
            }
            else if (angle <= 300) {
                realMotorAngle = constrain(angle, DIR_ANGLE_MIN, DIR_ANGLE_MAX);
                updateRealCurvature();
            }
            else {
                ret = DIR_STATUS_FAILURE;
            }
        }
        else {
            updateAimAngle();
            if (directionMotor.goalPositionDegree(aimMotorAngle) != OW_STATUS_OK) {
                ret = DIR_STATUS_FAILURE;
            }
        }
        read = !read;
    }

    if (ret != DIR_STATUS_OK) {
        Server.printf_err("DirectionController: communication error\n");
        return ret;
    }

    if (directionMotor.error()) {
        Server.printf_err("DirectionController: AX12 status %u\n",
            (uint8_t)directionMotor.status());
    }

    if (!blocked && directionMotor.environmentError()) {
        setBlocked(true);
    }

    return ret;
}

uint16_t DirectionController::getMotorAngle() const
{
    return realMotorAngle;
}

void DirectionController::setMotorAngle(uint16_t angle)
{
    aimMotorAngle = constrain(angle, DIR_ANGLE_MIN, DIR_ANGLE_MAX);
    float new_aimCurvature = angleToCurvature(aimMotorAngle);
    noInterrupts();
    aimCurvature = new_aimCurvature;
    interrupts();
}

size_t DirectionController::printTo(Print& p) const
{
    float aim, real;
    noInterrupts();
    aim = aimCurvature;
    real = realCurvature;
    interrupts();

    return p.printf("%u_%g_%g", millis(), aim, real);
}

void DirectionController::setAimCurvature(float curvature)
{
    aimCurvature = curvature;
}

float DirectionController::getRealCurvature() const
{
    return realCurvature;
}

bool DirectionController::isBlocked() const
{
    return blocked;
}

void DirectionController::updateRealCurvature()
{
    float new_realCurvature = angleToCurvature(realMotorAngle);
    noInterrupts();
    realCurvature = new_realCurvature;
    interrupts();
}

void DirectionController::updateAimAngle()
{
    noInterrupts();
    float aimCurvature_cpy = aimCurvature;
    interrupts();
    aimMotorAngle = curvatureToAngle(aimCurvature_cpy);
}

void DirectionController::setBlocked(bool b)
{
    noInterrupts();
    blocked = b;
    interrupts();
    blockedTimer = millis();
}

float DirectionController::angleToCurvature(uint16_t angle)
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

uint16_t DirectionController::curvatureToAngle(float curvature)
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
