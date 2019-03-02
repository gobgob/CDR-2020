#ifndef STOPPING_MGR_h
#define STOPPING_MGR_h

#include <Printable.h>
#include "Utils.h"
#include "Position.h"
#include "Average.h"

#define BREAKING_THRESHOLD  (0.2)   // BREAKING_THRESHOLD * FREQ_ASSERV s'exprimme en mm*s^-2


class StoppingMgr : public Printable
{
public:
	StoppingMgr(volatile float const & speed) :
		speed(speed)
	{
		epsilon = 0;
		responseTime = 0;
		stopped = false;
		moveBegin = false;
		beginTime = 0;
		breaking = false;
        abs_speed = 0;
        last_abs_speed = 0;
	}

	inline void compute()
	{
		abs_speed = ABS(speed);
		if (abs_speed < epsilon)
		{
			if (!stopped)
			{
				stopped = true;
				beginTime = millis();
			}
		}
		else
		{
			stopped = false;
			moveBegin = false;
		}
		averageAcceleration.add(abs_speed - last_abs_speed);
		last_abs_speed = abs_speed;
	}

	void moveIsStarting()
	{
		stopped = false;
		moveBegin = true;
	}

	void setTunings(float epsilon, uint32_t responseTime)
	{
		this->epsilon = epsilon;
		this->responseTime = responseTime;
	}

	void getTunings(float & epsilon, uint32_t & responseTime) const
	{
		epsilon = this->epsilon;
		responseTime = this->responseTime;
	}

	bool isStopped() const
	{
		if (moveBegin)
		{
			return stopped && millis() - beginTime > responseTime * 5;
		}
		else
		{
			return stopped && millis() - beginTime > responseTime;
		}
	}

	bool isMoveBegin() const
	{
		return moveBegin;
	}

	/* Indique si on est en train de ralentir */
	bool isBreaking() const
	{
		return averageAcceleration.value() < -BREAKING_THRESHOLD;
	}

	size_t printTo(Print& p) const
	{
		return p.printf("%u_%g_%d", millis(), speed, isStopped());
	}

private:
	volatile float const & speed;
	
	float epsilon;
	uint32_t responseTime; // ms

	uint32_t beginTime;
	bool stopped;
	bool moveBegin;
	float abs_speed;
	float last_abs_speed;
	bool breaking;
	Average<float, 50> averageAcceleration;
};


#endif
