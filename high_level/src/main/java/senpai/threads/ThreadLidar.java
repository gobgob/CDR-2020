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
import pfg.kraken.utils.XY;
import pfg.log.Log;
import senpai.comm.LidarEth;
import senpai.obstacles.ObstaclesDynamiques;
import senpai.robot.Robot;
import senpai.robot.RobotColor;
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
	private double multiplier;

	public ThreadLidar(LidarEth eth, ObstaclesDynamiques dynObs, Robot robot, Log log, Config config)
	{
		this.eth = eth;
		this.config = config;
		this.log = log;
		this.dynObs = dynObs;
		this.robot = robot;
		multiplier = config.getDouble(ConfigInfoSenpai.SLOW_OBSTACLE_RADIUS_MULTIPLIER);
		setDaemon(true);
	}

	@Override
	public void run()
	{
		boolean colorSent = false;
		boolean started = false;
		boolean stopped = false;
		
		Thread.currentThread().setName(getClass().getSimpleName());
		try
		{
			if(!config.getBoolean(ConfigInfoSenpai.ENABLE_LIDAR))
			{
				log.write("Démarrage de " + Thread.currentThread().getName()+" annulé !", Severity.WARNING, Subject.STATUS);
				while(true)
					Thread.sleep(10);
			}
			else
			{
				log.write("Démarrage de " + Thread.currentThread().getName(), Subject.STATUS);	
				eth.initialize(config);
				while(true)
				{
					String message = eth.getMessage();

					if(message.startsWith("ASK_STATUS"))
					{
						if(!colorSent)
						{
							RobotColor c = robot.getColor();
							if(c != null)
							{
								eth.sendInit(c);
								colorSent = true;
							}
						}
						else if(!started && robot.isMatchStarted())
						{
							eth.sendStart();
							started = true;
						}
						else if(!stopped && robot.isMatchStopped())
						{
							eth.sendStop();
							stopped = true;
						}
						else
							eth.sendAck();
					}
					else if(message.startsWith("OBSTACLE"))
					{
						String[] m = message.split(" ");
						if(m.length == 6)
						{
							int x = Integer.parseInt(m[1]);
							int y = Integer.parseInt(m[2]);
							int rad = Integer.parseInt(m[3]);
							int id = Integer.parseInt(m[4]);
							int radSlow = (int) Math.round(multiplier * rad);
//							int timestamp = Integer.parseInt(m[5]);
							assert id < 100 : id;
							
							CircularObstacle obs = new CircularObstacle(new XY(x, y), rad);
							dynObs.setLidarObs(obs, id);
							
							CircularObstacle obsSlow = new CircularObstacle(new XY(x, y), radSlow);
							robot.setLidarObs(obsSlow, id);
							
							eth.sendAck();
						}
						else
							log.write("Message malformé provenant du Lidar: " + message, Severity.CRITICAL, Subject.CAPTEURS);
							

					}
					else
					{
						assert false : message;
						log.write("Message inconnu provenant du Lidar: " + message, Severity.CRITICAL, Subject.CAPTEURS);
					}

				}
			}
		}
		catch(InterruptedException e)
		{
			eth.close();
			robot.clearLidarObs();
			dynObs.clearLidarObs();
			log.write("Arrêt de " + Thread.currentThread().getName(), Subject.STATUS);
			Thread.currentThread().interrupt();
		}
		catch(Exception e)
		{
			eth.close();
			robot.clearLidarObs();
			dynObs.clearLidarObs();
			log.write("Arrêt inattendu de " + Thread.currentThread().getName() + " : " + e, Severity.CRITICAL, Subject.STATUS);
			e.printStackTrace();
			Thread.currentThread().interrupt();
		}
	}

}