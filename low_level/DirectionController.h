#pragma once

#include <Printable.h>
#include <DynamixelAX12.h>
#include <stdint.h>
#include "Singleton.h"

#define DIR_INFINITE_CURVATURE	(1e6f)


enum DirectionControllerStatus
{
    DIR_STATUS_OK = 0,
    DIR_STATUS_NOT_UPDATED,
    DIR_STATUS_FAILURE,
};


class DirectionController : public Singleton<DirectionController>,
    public Printable
{
public:
    DirectionController();

    DirectionControllerStatus init();
    DirectionControllerStatus control();
    uint16_t getMotorAngle() const;
    void setMotorAngle(uint16_t angle);
    size_t printTo(Print& p) const;

    /*--- Méthodes à appeller depuis une interruption ---*/
    void setAimCurvature(float curvature);
    float getRealCurvature() const;
    bool isBlocked() const;
    /*--- fin ---*/

private:
    void updateRealCurvature();
    void updateAimAngle();
    void setBlocked(bool b);

    static float angleToCurvature(uint16_t angle);
    static uint16_t curvatureToAngle(float curvature);
    
    /* Courbure, en m^-1 */
    volatile float aimCurvature;
    volatile float realCurvature;

    /* Angle de l'AX12, en degrés */
    uint16_t aimMotorAngle;
    uint16_t realMotorAngle;

    /* L'AX12 de direction */
    DynamixelAX12 directionMotor;

    /* Indique si l'AX12 est en mesure de bouger ou non */
    volatile bool blocked;
    uint32_t blockedTimer; // ms

    /* Indique si la méthode init a été exécutée avec succès */
    bool is_init;
};
