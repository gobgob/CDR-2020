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

/**
 * Thread qui gère les données du lidar
 * 
 * @author pf
 *
 */
/*
public class ThreadLidar extends Thread
{
	protected ObstaclesDynamiques dynObs;
	protected Config config;
	protected Robot robot;
	protected Log log;
	protected LidarEth eth;
	private double multiplier;
	private int radius;
	private int timeout;
	private volatile long lastMessageDate = Long.MAX_VALUE;
	private Process lidarProcess = null;
	private String command;
	private boolean enableLidarScript;

	public ThreadLidar(LidarEth eth, ObstaclesDynamiques dynObs, Robot robot, Log log, Config config)
	{
		this.eth = eth;
		this.config = config;
		this.log = log;
		this.dynObs = dynObs;
		this.robot = robot;
		multiplier = config.getDouble(ConfigInfoSenpai.SLOW_OBSTACLE_RADIUS_MULTIPLIER);
		radius = config.getInt(ConfigInfoSenpai.LIDAR_OBSTACLE_RADIUS);
		timeout = config.getInt(ConfigInfoSenpai.TIMEOUT_LIDAR);
		enableLidarScript = config.getBoolean(ConfigInfoSenpai.ENABLE_LIDAR_SCRIPT);
		command = config.getString(ConfigInfoSenpai.LIDAR_COMMAND);
		setDaemon(true);
	}

	@Override
	public void run()
	{
		boolean colorSent = false;
		boolean started = false;
		boolean stopped = false;
//		long lastCorrection = 0;
		
		Thread.currentThread().setName(getClass().getSimpleName());
		try
		{
			if(!config.getBoolean(ConfigInfoSenpai.ENABLE_LIDAR))
			{
				log.write("Démarrage de " + Thread.currentThread().getName()+" annulé !", Severity.WARNING, Subject.STATUS);
				while(true)
					Thread.sleep(100);
			}
			else
			{
				log.write("Démarrage de " + Thread.currentThread().getName(), Subject.STATUS);	
				eth.initialize(config);
				
				if(enableLidarScript)
				{
					log.write("Démarrage du script " + command, Subject.STATUS);	
					lidarProcess = Runtime.getRuntime().exec(command);
				}
				
				eth.waitLidar();

				while(true)
				{
					String message = eth.getMessage();
					
					if(isInterrupted())
					{
						log.write("Timeout lidar !", Subject.STATUS);
						while(true)
							Thread.sleep(100);
					}
					
					if(message != null)
						log.write("Message lidar : "+message, Subject.COMM);	
					else
						throw new IOException("Déconnexion prématurée du lidar");
					log.write("LidarCorr: "+robot.printLidarStatus(), Subject.COMM);
					lastMessageDate = System.currentTimeMillis();
					
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
							else
								eth.sendAck(); // rien à dire
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
						else if(robot.needLidarCorrection() && robot.isLidarCorrectionAllowed())
						{
							robot.startLidarCorrection();
							eth.sendOdo();
						}
						else
							eth.sendAck();
					}
					else if(message.startsWith("OBSTACLE"))
					{
						String[] m = message.split(" ");
						if(m.length == 5)
						{
							int x = Integer.parseInt(m[1]);
							int y = Integer.parseInt(m[2]);
							int id = Integer.parseInt(m[3]);
							int radSlow = (int) Math.round(multiplier * radius);
							assert id < 100 : id;
							CircularObstacle obs = new CircularObstacle(new XY(x, y), radius);
							dynObs.setLidarObs(obs, id);
							
							CircularObstacle obsSlow = new CircularObstacle(new XY(x, y), radSlow);
							robot.setLidarObs(obsSlow, id);
							
							eth.sendAck();
						}
						else
							log.write("Message malformé provenant du Lidar: " + message, Severity.CRITICAL, Subject.CAPTEURS);
					}
					else if(message.startsWith("DECALAGE_ERREUR"))
					{
						log.write("Erreur dans le recalage du lidar ! " + message, Severity.CRITICAL, Subject.CAPTEURS);
						robot.stopLidarCorrection();
						eth.sendAck();
					}
					else if(message.startsWith("DECALAGE"))
					{
						String[] m = message.split(" ");
						if(m.length == 4)
						{
							boolean allowed = robot.isLidarCorrectionAllowed();
							boolean timeout = robot.isLidarTimeout(); 
							if(allowed && !timeout)
							{
								XYO correction = new XYO(Integer.parseInt(m[1]), Integer.parseInt(m[2]), Double.parseDouble(m[3]));
								if(Math.abs(correction.orientation) > 15*Math.PI/180)
								{
									log.write("Correction Lidar trop grande ! "+correction, Severity.WARNING, Subject.STATUS);
								}
								else
								{
									log.write("Envoi d'une correction XYO lidar: " + correction, Subject.STATUS);
									robot.correctPosition(correction.position, correction.orientation);
								}
							}
							else if(!allowed)
								log.write("Correction d'odo par lidar annulée : correction par capteurs trop récente.", Subject.STATUS);
							else
								log.write("Correction d'odo par lidar annulée : durée trop longue entre request et réponse du lidar.", Subject.STATUS);
							robot.stopLidarCorrection();
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
		catch(InterruptedException | SocketException e)
		{
			robot.disableLidar();
			eth.close();
			if(lidarProcess != null)
				lidarProcess.destroy();
			robot.clearLidarObs();
			dynObs.clearLidarObs();
			log.write("Arrêt de " + Thread.currentThread().getName(), Subject.STATUS);
			Thread.currentThread().interrupt();
		}
		catch(Exception e)
		{
			robot.disableLidar();
			eth.close();
			if(lidarProcess != null)
				lidarProcess.destroy();
			robot.clearLidarObs();
			dynObs.clearLidarObs();
			log.write("Arrêt inattendu de " + Thread.currentThread().getName() + " : " + e, Severity.CRITICAL, Subject.STATUS);
			e.printStackTrace();
			Thread.currentThread().interrupt();
		}
	}
	
	public boolean timeout()
	{
		return System.currentTimeMillis() - lastMessageDate > timeout;
	}

	public void close()
	{
		eth.close();
	}

}*/