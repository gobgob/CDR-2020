/*
 * Copyright (C) 2013-2018 Pierre-François Gimenez
 * Distributed under the MIT License.
 */

package senpai.threads;

import pfg.log.Log;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.obstacles.ObstaclesDynamiques;
import senpai.robot.Robot;
import senpai.utils.Severity;
import senpai.utils.Subject;

import java.util.ArrayList;
import java.util.List;

import pfg.kraken.obstacles.RectangularObstacle;
import pfg.kraken.robot.ItineraryPoint;

/**
 * Thread qui s'occupe de la détection de collisions lors des déplacements
 * 
 * @author pf
 *
 */

public final class ThreadCollision extends Thread
{
	protected Log log;
	private ObstaclesDynamiques dynObs;
	private Robot robot;
	private OutgoingOrderBuffer out;
	private List<RectangularObstacle> initialObstacles = new ArrayList<RectangularObstacle>();
	private RectangularObstacle[] memory;


	public ThreadCollision(Log log, ObstaclesDynamiques dynObs, Robot robot, OutgoingOrderBuffer out, RectangularObstacle vehicleTemplate)
	{
		this.dynObs = dynObs;
		this.log = log;
		this.robot = robot;
		this.out = out;
		memory = new RectangularObstacle[2000];
		for(int i = 0; i < memory.length; i++)
			memory[i] = vehicleTemplate.clone();
		setDaemon(true);
	}

	@Override
	public void run()
	{
		Thread.currentThread().setName(getClass().getSimpleName());
		log.write("Démarrage de " + Thread.currentThread().getName(), Subject.STATUS);
		try
		{
			List<ItineraryPoint> currentPath;
			while(true)
			{
				synchronized(robot)
				{
					while(!robot.needCollisionCheck())
						robot.wait();

					currentPath = robot.getPath();
				}				

				initialObstacles.clear();
				int i = 0;
				for(ItineraryPoint ip : currentPath)
				{
					memory[i].update(ip.x, ip.y, ip.orientation);
					initialObstacles.add(memory[i]);
					i++;
				}
				
				dynObs.clearNew();

				/*
				 * On attend d'avoir des obstacles à vérifier
				 */
				synchronized(dynObs)
				{
//					log.write("Attente obstacles.", Subject.STATUS);

					while(!dynObs.needCollisionCheck())
						dynObs.wait();
					
//					log.write("Obstacle !", Subject.STATUS);

				}

				synchronized(robot)
				{
					int currentIndexTrajectory = robot.getIndexTrajectory();
//					log.write("Vérification collision.", Subject.STATUS);					

					int indexMax = Math.min(currentIndexTrajectory + 13, initialObstacles.size());
					if(indexMax > currentIndexTrajectory)
					{
						List<RectangularObstacle> subList = initialObstacles.subList(currentIndexTrajectory, indexMax);
						
						if(robot.needCollisionCheck() && dynObs.isThereCollision(subList) != subList.size())
						{
							robot.setStopping();
							out.immobilise();
						}
					}
//					log.write("Vérification finie.", Subject.STATUS);

				}
			}
		}
		catch(InterruptedException e)
		{
			log.write("Arrêt de " + Thread.currentThread().getName(), Subject.STATUS);
			Thread.currentThread().interrupt();
		}
		catch(Exception e)
		{
			log.write("Arrêt inattendu de " + Thread.currentThread().getName() + " : " + e, Severity.CRITICAL, Subject.STATUS);
			e.printStackTrace();
			Thread.currentThread().interrupt();
		}
	}

}
