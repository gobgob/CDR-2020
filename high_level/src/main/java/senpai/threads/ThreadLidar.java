/*
 * Copyright (C) 2013-2018 Pierre-François Gimenez
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>
 */

package senpai.threads;

import pfg.config.Config;
import pfg.kraken.obstacles.CircularObstacle;
import pfg.log.Log;
import senpai.comm.LidarEth;
import senpai.obstacles.ObstaclesDynamiques;
import senpai.robot.Robot;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Severity;
import senpai.utils.Subject;

/**
 * Thread qui gère les données du lidar
 * 
 * @author pf
 *
 */

public class ThreadLidar extends Thread
{
	protected ObstaclesDynamiques dynObs;
	protected Config config;
	protected Robot robot;
	protected Log log;
	protected LidarEth eth;

	public ThreadLidar(LidarEth eth, ObstaclesDynamiques dynObs, Robot robot, Log log, Config config)
	{
		this.eth = eth;
		this.config = config;
		this.log = log;
		this.dynObs = dynObs;
		this.robot = robot;
		setDaemon(true);
	}

	@Override
	public void run()
	{
		Thread.currentThread().setName(getClass().getSimpleName());
		try
		{
			if(!config.getBoolean(ConfigInfoSenpai.ENABLE_LIDAR))
			{
				log.write("Démarrage de " + Thread.currentThread().getName()+" annulé !", Severity.WARNING, Subject.STATUS);
				while(true)
				{
					CircularObstacle obs = eth.getObstacle();
					if(obs != null)
					{
						dynObs.setLidarObs(obs);
						robot.setLidarObs(obs);
					}
				}
			}
			else
			{
				log.write("Démarrage de " + Thread.currentThread().getName(), Subject.STATUS);	
				while(true)
					Thread.sleep(10);
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
			robot.clearLidarObs();
			dynObs.clearLidarObs();
			e.printStackTrace();
			Thread.currentThread().interrupt();
		}
	}

}