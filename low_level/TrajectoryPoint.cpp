#include "TrajectoryPoint.h"
#include <Arduino.h>

TrajectoryPoint::TrajectoryPoint()
{
	/* Default TrajectoryPoint */
	stopPoint = true;
	endOfTrajectory = true;
	curvature = 0;
	algebricMaxSpeed = 0;
}

TrajectoryPoint::TrajectoryPoint(const Position& aPos, float aCurvature,
	float aSpeed, bool isStopPoint, bool isEndOfTraj)
{
	position = aPos;
	curvature = aCurvature;
	algebricMaxSpeed = aSpeed;
	stopPoint = isStopPoint;
	endOfTrajectory = isEndOfTraj;
}

size_t TrajectoryPoint::printTo(Print& p) const
{
	size_t count = 0;
	count += p.print(position);
	count += p.printf("_%g_%g_%d_%d", curvature, algebricMaxSpeed, stopPoint,
		endOfTrajectory);
	return count;
}
