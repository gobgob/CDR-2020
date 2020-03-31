#include "PID.h"
#include <Arduino.h>

PID::PID(volatile float const& input, volatile float& output,
	volatile float const& setPoint, const float freqAsserv) :
	input(input),
	output(output),
	setPoint(setPoint),
	callFreq(freqAsserv)
{
	setOutputLimits(-MAXFLOAT, MAXFLOAT);
	setTunings(0, 0, 0);
	pre_error = 0;
	derivative = 0;
	integral = 0;
	resetDerivativeError();
}

void PID::compute()
{
	float error = setPoint - input;
	derivative = (error - pre_error) * callFreq;
	integral += error / callFreq;
	pre_error = error;

	float result = kp * error + ki * integral + kd * derivative;
	if (result > outMax) {
		result = outMax;
	}
	else if (result < outMin) {
		result = outMin;
	}
	output = result;
}

void PID::setTunings(float kp, float ki, float kd)
{
	if (kp < 0 || ki < 0 || kd < 0) {
		this->kp = 0;
		this->ki = 0;
		this->kd = 0;
	}
	this->kp = kp;
	this->ki = ki;
	this->kd = kd;
}

void PID::setOutputLimits(float min, float max) {
	if (min >= max) {
		return;
	}

	outMin = min;
	outMax = max;

	if (output > outMax) {
		output = outMax;
	}
	else if (output < outMin) {
		output = outMin;
	}
}

void PID::resetDerivativeError()
{
	pre_error = setPoint - input; // pre_error = error
	derivative = 0;
}

void PID::resetIntegralError()
{
	integral = 0;
}

size_t PID::printTo(Print& p) const
{
	float in, out, sp;
	noInterrupts();
	in = input;
	out = output;
	sp = setPoint;
	interrupts();

	return p.printf("%u_%g_%g_%g", millis(), in, out, sp);
}
