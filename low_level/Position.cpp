#include "Position.h"
#include <Arduino.h>
#include "Utils.h"

Position::Position()
{
	x = 0;
	y = 0;
	orientation = 0;
}

Position::Position(float _x, float _y, float _o)
{
	x = _x;
	y = _y;
	setOrientation(_o);
}

void Position::operator= (volatile const Position& newPosition) volatile
{
	this->x = newPosition.x;
	this->y = newPosition.y;
	this->orientation = newPosition.orientation;
}

bool Position::isCloserToAThanB(Position const& positionA,
	Position const& positionB) const volatile
{
	float squaredDistanceToA = square(x - positionA.x) + square(y - positionA.y);
	float squaredDistanceToB = square(x - positionB.x) + square(y - positionB.y);
	return squaredDistanceToA <= squaredDistanceToB;
}

float Position::distanceTo(Position const& pos) const volatile
{
	return sqrtf(square(pos.x - x) + square(pos.y - y));
}

void Position::setOrientation(float _o)
{
	orientation = fmodulo(_o, TWO_PI);
}

void Position::setOrientation(float _o) volatile
{
	orientation = fmodulo(_o, TWO_PI);
}

size_t Position::printTo(Print& p) const
{
	return p.printf("%g_%g_%g", x, y, orientation);
}
