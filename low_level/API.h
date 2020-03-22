#ifndef _API_h
#define _API_h

#include "OrderMacros.h"
#include "OrderImmediate.h"
#include "OrderLong.h"
#include "Singleton.h"
#include "MoveState.h"

ORDER_IMMEDIATE(Ping, 0);
ORDER_IMMEDIATE(GetColor, 0);
ORDER_IMMEDIATE(EditPosition, 12);
ORDER_IMMEDIATE(SetPosition, 12);
ORDER_IMMEDIATE(AppendToTraj, VARIABLE_INPUT_SIZE);
ORDER_IMMEDIATE(EditTraj, VARIABLE_INPUT_SIZE);
ORDER_IMMEDIATE(DeleteTrajPts, 4);
ORDER_IMMEDIATE(SetScore, 4);

ORDER_IMMEDIATE(Display, 0);
ORDER_IMMEDIATE(Save, 0);
ORDER_IMMEDIATE(LoadDefaults, 0);
ORDER_IMMEDIATE(GetPosition, 0);
ORDER_IMMEDIATE(SetControlLevel, 1);
ORDER_IMMEDIATE(StartManualMove, 0);
ORDER_IMMEDIATE(SetMaxSpeed, 4);
ORDER_IMMEDIATE(SetAimDistance, 4);
ORDER_IMMEDIATE(SetCurvature, 4);
ORDER_IMMEDIATE(SetDirAngle, 4);
ORDER_IMMEDIATE(SetTranslationTunings, 12);
ORDER_IMMEDIATE(SetTrajectoryTunings, 12);
ORDER_IMMEDIATE(SetStoppingTunings, 8);
ORDER_IMMEDIATE(SetMaxAcceleration, 4);
ORDER_IMMEDIATE(SetMaxDeceleration, 4);
ORDER_IMMEDIATE(SetMaxCurvature, 4);

ORDER_LONG(FollowTrajectory, 0, MoveStatus status;);
ORDER_LONG(Stop, 0, );
ORDER_LONG(WaitForJumper, 0, JumperState state; uint32_t debounceTimer;);
ORDER_LONG(StartChrono, 0, uint32_t chrono;);

#endif
