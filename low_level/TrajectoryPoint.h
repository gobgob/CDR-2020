#pragma once

#include <Printable.h>
#include "Position.h"

class TrajectoryPoint : public Printable
{
public:
	TrajectoryPoint();
	TrajectoryPoint(const Position& aPos, float aCurvature, float aSpeed,
		bool isStopPoint, bool isEndOfTraj);

	const Position& getPosition() const { return position; }
	bool isStopPoint() const { return stopPoint; }
	bool isEndOfTrajectory() const { return endOfTrajectory; }
	float getCurvature() const { return curvature; }
	float getAlgebricMaxSpeed() const { return algebricMaxSpeed; }

	size_t printTo(Print& p) const;

private:
	Position position;
	bool stopPoint;
	bool endOfTrajectory;
	float curvature; // m^-1
	float algebricMaxSpeed; // mm/s
};
