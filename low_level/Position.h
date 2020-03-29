#pragma once

#include <Printable.h>

class Position : public Printable
{
public:
	Position();
	Position(float _x, float _y, float _o);
	void operator= (volatile const Position& newPosition) volatile;

	bool isCloserToAThanB(Position const& positionA,
		Position const& positionB) const volatile;
	float distanceTo(Position const& pos) const volatile;
	void setOrientation(float _o);
	void setOrientation(float _o) volatile;

	size_t printTo(Print& p) const;

	float x; // mm
	float y; // mm
	float orientation; // radians
};
