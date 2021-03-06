#ifndef _MOTIONCONTROLSYSTEM_h
#define _MOTIONCONTROLSYSTEM_h

#include "TrajectoryFollower.h"
#include "MoveState.h"
#include "Position.h"
#include "TrajectoryPoint.h"
#include "MotionControlTunings.h"
#include "Singleton.h"
#include "CommunicationServer.h"
#include <vector>


#define FREQ_ASSERV		1000					// Fr�quence d'asservissement (Hz)
#define PERIOD_ASSERV	(1000000 / FREQ_ASSERV)	// P�riode d'asservissement (�s)
#define TRAJECTORY_STEP 20			            // Distance entre deux points d'une trajectoire (mm)

#define TRAJECTORY_EDITION_SUCCESS  0
#define TRAJECTORY_EDITION_FAILURE  1


class MotionControlSystem : public Singleton<MotionControlSystem>
{
public:
	MotionControlSystem() :
		trajectoryFollower(FREQ_ASSERV, position, moveStatus)
	{
		travellingToDestination = false;
		trajectoryIndex = 0;
		trajectoryComplete = false;
		moveStatus = MOVE_OK;
	}


	/*
		#################################################
		#  M�thodes � appeller durant une interruption  #
		#################################################
	*/

	void control()
	{
		static bool wasTravellingToDestination = false;

        trajectoryFollower.control();
		if (travellingToDestination)
		{
            MovePhase movePhase = trajectoryFollower.getMovePhase();

            if (currentTrajectory.size() > trajectoryIndex)
            {
			    if (!wasTravellingToDestination)
			    {// D�marrage du suivi de trajectoire
				    trajectoryFollower.setTrajectoryPoint(currentTrajectory.at(trajectoryIndex));
                    updateDistanceToTravel();
				    trajectoryFollower.startMove();
				    wasTravellingToDestination = true;
			    }
                else if (movePhase == MOVING && !currentTrajectory.at(trajectoryIndex).isStopPoint())
                {
                    Position trajPoint = currentTrajectory.at(trajectoryIndex).getPosition();
                    float sign;
                    if (trajectoryFollower.isMovingForward()) {
                        sign = 1;
                    }
                    else {
                        sign = -1;
                    }

                    if (((position.x - trajPoint.x) * cosf(trajPoint.orientation) + (position.y - trajPoint.y) * sinf(trajPoint.orientation)) * sign > 0)
                    {
                        if (currentTrajectory.size() > trajectoryIndex + 1)
                        {
                            trajectoryIndex++;
                            updateDistanceToTravel();
                            trajectoryFollower.setTrajectoryPoint(currentTrajectory.at(trajectoryIndex));
                        }
                        else
                        {
                            moveStatus |= EMPTY_TRAJ;
                            stop_and_clear_trajectory_from_interrupt();
                            Server.asynchronous_trace(__LINE__);
                        }
                    }
                }
                else if (movePhase == MOVE_ENDED)
                {
                    if (currentTrajectory.at(trajectoryIndex).isEndOfTrajectory() || moveStatus != MOVE_OK)
                    {
                        stop_and_clear_trajectory_from_interrupt();
                        travellingToDestination = false;
                        wasTravellingToDestination = false;
                    }
                    else
                    {
                        if (currentTrajectory.size() > trajectoryIndex + 1)
                        {
                            trajectoryIndex++;
                            updateDistanceToTravel();
                            trajectoryFollower.setTrajectoryPoint(currentTrajectory.at(trajectoryIndex));
                            trajectoryFollower.startMove();
                        }
                        else
                        {
                            moveStatus |= EMPTY_TRAJ;
                            stop_and_clear_trajectory_from_interrupt();
                            travellingToDestination = false;
                            wasTravellingToDestination = false;
                            Server.asynchronous_trace(__LINE__);
                        }
                    }
                }
            }
            else if (movePhase != BREAKING)
            {
                stop_and_clear_trajectory_from_interrupt();
                if (movePhase != MOVE_ENDED || !wasTravellingToDestination)
                {
                    moveStatus |= EMPTY_TRAJ;
                }
                travellingToDestination = false;
                wasTravellingToDestination = false;
            }
		}
	}

private:
    void stop_and_clear_trajectory_from_interrupt()
    {
        // todo (v�rifier que j'ai pens�  tout)
        trajectoryFollower.emergency_stop_from_interrupt();
        trajectoryIndex = 0;
        currentTrajectory.clear();
        trajectoryComplete = false;
    }

    void updateDistanceToTravel()
    {
        size_t i;
        for (i = trajectoryIndex; i < currentTrajectory.size(); i++)
        {
            if (currentTrajectory.at(i).isStopPoint())
            {
                float distanceToDrive = ((float)i - (float)trajectoryIndex + 1) * TRAJECTORY_STEP;
                trajectoryFollower.setDistanceToDrive(distanceToDrive);
                return;
            }
        }
        trajectoryFollower.setInfiniteDistanceToDrive();
    }


	/*
		###################################################
		#  M�thodes � appeller dans la boucle principale  #
		###################################################
	*/

 public:
	void followTrajectory()
	{
        if (trajectoryFollower.isTrajectoryControlled())
        {
            noInterrupts();
            moveStatus = MOVE_OK;
            travellingToDestination = true;
            interrupts();
        }
        else
        {
            Server.printf_err("MotionControlSystem::followTrajectory : trajectory is not controlled\n");
        }
	}

    void startManualMove()
    {
        if (!trajectoryFollower.isTrajectoryControlled())
        {
            noInterrupts();
            moveStatus = MOVE_OK;
            trajectoryFollower.startMove();
            interrupts();
            Server.printf("Manual move started");
        }
        else
        {
            Server.printf_err("MotionControlSystem::startManualMove : trajectory is controlled\n");
        }
    }

	void stop_and_clear_trajectory()
	{
        uint32_t t = millis();
        Position p = getPosition();
        noInterrupts();
        stop_and_clear_trajectory_from_interrupt();
        interrupts();
        Server.printf("Emergency stop started at: %u\n", t);
        Server.printf("pos= ");
        Server.println(p);
	}

	bool isMovingToDestination() const
	{
		noInterrupts();
		bool moving = travellingToDestination;
		interrupts();
		return moving;
	}

	uint8_t appendToTrajectory(TrajectoryPoint trajectoryPoint)
	{
		if (!trajectoryComplete)
		{
			noInterrupts();
			currentTrajectory.push_back(trajectoryPoint);
			interrupts();
			if (trajectoryPoint.isEndOfTrajectory())
			{
				trajectoryComplete = true;
			}
            Position p = trajectoryPoint.getPosition();
            Server.printf(AIM_TRAJECTORY, "%u_%g_%g", millis(), p.x, p.y);
            return TRAJECTORY_EDITION_SUCCESS;
		}
		else
		{
            return TRAJECTORY_EDITION_FAILURE;
		}
	}

	uint8_t updateTrajectory(size_t index, TrajectoryPoint trajectoryPoint)
	{
		if (index < currentTrajectory.size() && index >= trajectoryIndex)
		{
			if (index == trajectoryIndex && travellingToDestination)
			{
                return TRAJECTORY_EDITION_FAILURE;
			}
			else
			{
				noInterrupts();
                if (currentTrajectory.at(index).isEndOfTrajectory())
                {
                    trajectoryComplete = false;
                }
				currentTrajectory.at(index) = trajectoryPoint;
                if (trajectoryPoint.isEndOfTrajectory())
                {
                    trajectoryComplete = true;
                    if (currentTrajectory.size() > index + 1)
                    {
                        currentTrajectory.erase(currentTrajectory.begin() + index + 1, currentTrajectory.end());
                    }
                }
				interrupts();

                return TRAJECTORY_EDITION_SUCCESS;
			}
		}
		else
		{
            return TRAJECTORY_EDITION_FAILURE;
		}
	}

    uint8_t deleteTrajectoryPoints(size_t index)
    {
        if (index < currentTrajectory.size() && index >= trajectoryIndex)
        {
            if (index == trajectoryIndex && travellingToDestination)
            {
                return TRAJECTORY_EDITION_FAILURE;
            }
            else
            {
                noInterrupts();
                trajectoryComplete = false;
                currentTrajectory.erase(currentTrajectory.begin() + index, currentTrajectory.end());
                interrupts();
                return TRAJECTORY_EDITION_SUCCESS;
            }
        }
        else
        {
            return TRAJECTORY_EDITION_FAILURE;
        }
    }

	Position getPosition() const
	{
		static Position p;
		noInterrupts();
		p = position;
		interrupts();
		return p;
	}

	void setPosition(Position p)
	{
		noInterrupts();
		position = p;
		interrupts();
	}

	MoveStatus getMoveStatus() const
	{
		static MoveStatus ms;
		noInterrupts();
		ms = moveStatus;
		interrupts();
		return ms;
	}

    size_t getTrajectoryIndex() const
    {
        noInterrupts();
        size_t ret = trajectoryIndex;
        interrupts();
        return ret;
    }

	void setMotionControlLevel(uint8_t level)
	{
		trajectoryFollower.setMotionControlLevel(level);
	}

	uint8_t getMotionControlLevel() const
	{
		return trajectoryFollower.getMotionControlLevel();
	}

	void setTunings(MotionControlTunings const & tunings)
	{
		trajectoryFollower.setTunings(tunings);
	}

    void enableHighSpeed(bool enable)
    {
        if (enable) {
            trajectoryFollower.setHighSpeedTunings();
        }
        else {
            trajectoryFollower.setDefaultTunings();
        }
    }

	MotionControlTunings getTunings() const
	{
		return trajectoryFollower.getTunings();
	}

    void setMaxSpeed(float speed)
    {
        noInterrupts();
        trajectoryFollower.setMaxSpeed(speed);
        interrupts();
    }

    void setDistanceToDrive(float distance)
    {
        noInterrupts();
        trajectoryFollower.setDistanceToDrive(distance);
        interrupts();
    }

    void setCurvature(float curvature)
    {
        noInterrupts();
        trajectoryFollower.setCurvature(curvature);
        interrupts();
    }

    float getCurvature() const
    {
        noInterrupts();
        float curvature = trajectoryFollower.getCurvature();
        interrupts();
        return curvature;
    }

    void sendLogs()
    {
        trajectoryFollower.sendLogs();
    }

    bool isMovingForward() const
    {
        noInterrupts();
        bool ret = trajectoryFollower.isMovingForward();
        interrupts();
        return ret;
    }

    int getMovingDirection() const
    {
        noInterrupts();
        MovePhase mp = trajectoryFollower.getMovePhase();
        interrupts();
        if (mp == MOVE_ENDED) {
            return 0;
        }
        else if (isMovingForward()) {
            return 1;
        }
        else {
            return -1;
        }
    }

    bool isBreaking() const
    {
        noInterrupts();
        bool ret = trajectoryFollower.isBreaking();
        interrupts();
        return ret;
    }

    float getMovingSpeed() const
    {
        noInterrupts();
        float ret = trajectoryFollower.getCurrentMovingSpeed();
        interrupts();
        return ret;
    }

    void enableParkingBreak(bool enable)
    {
        trajectoryFollower.enableParkingBreak(enable);
    }

    bool parkingBreakEnabled() const
    {
        return trajectoryFollower.parkingBreakEnabled();
    }

    void getRawTicks(int32_t &leftTicks, int32_t &rightTicks) const
    {
        trajectoryFollower.getRawTicks(leftTicks, rightTicks);
    }

private:
	TrajectoryFollower trajectoryFollower;
	volatile Position position;
	volatile MoveStatus moveStatus;
	volatile bool travellingToDestination;  // Indique si le robot est en train de parcourir la trajectoire courante
	volatile size_t trajectoryIndex;  // Point courant de la trajectoire courante

	std::vector<TrajectoryPoint> currentTrajectory;
	bool trajectoryComplete;
};


#endif
