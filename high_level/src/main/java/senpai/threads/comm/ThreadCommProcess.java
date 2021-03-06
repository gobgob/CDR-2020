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

package senpai.threads.comm;

import java.nio.ByteBuffer;
import pfg.kraken.struct.Kinematic;
import pfg.log.Log;
import senpai.Senpai;
import senpai.Senpai.ErrorCode;
import senpai.buffer.IncomingOrderBuffer;
import senpai.buffer.SensorsDataBuffer;
import senpai.capteurs.CapteursRobot;
import senpai.capteurs.SensorsData;
import senpai.comm.Paquet;
import senpai.comm.CommProtocol;
import senpai.comm.CommProtocol.Id;
import senpai.comm.CommProtocol.LLStatus;
import senpai.robot.Robot;
import senpai.robot.RobotColor;
import senpai.utils.Severity;
import senpai.utils.Subject;

/**
 * Thread qui écoute la série et appelle qui il faut.
 * 
 * @author pf
 *
 */

public class ThreadCommProcess extends Thread
{
	protected Log log;
	private IncomingOrderBuffer serie;
	private SensorsDataBuffer buffer;
	private Robot robot;
	private Senpai container;
	private Kinematic current = new Kinematic();
	private SensorsData[] memory;
	private int indexMem = 0;
//	private DynamicPath chemin;

	public volatile boolean capteursOn = false;
	private long dernierAffichagePosition = System.currentTimeMillis();

	public ThreadCommProcess(Log log, IncomingOrderBuffer serie, SensorsDataBuffer buffer, Robot robot, Senpai container/*, DynamicPath chemin*/)
	{
		this.container = container;
		this.log = log;
		this.serie = serie;
		this.buffer = buffer;
		this.robot = robot;

		memory = new SensorsData[100];
		for(int i = 0; i < memory.length; i++)
			memory[i] = new SensorsData();
		
//		this.chemin = chemin;
		setDaemon(true);
		setPriority(Thread.MAX_PRIORITY);
	}

	@Override
	public void run()
	{
		Thread.currentThread().setName(getClass().getSimpleName());
		log.write("Démarrage de " + Thread.currentThread().getName(), Subject.STATUS);

		try
		{
			while(true)
			{
//				long avant = System.currentTimeMillis();

				Paquet paquet = serie.take();

//				long duree = (System.currentTimeMillis() - avant);
//				log.write("Durée avant obtention du paquet : " + duree + ". Traitement de " + paquet, Subject.COMM);

				long avant = System.currentTimeMillis();
				ByteBuffer data = paquet.message;
				
				/**
				 * Couleur du robot
				 */
				if(paquet.origine == Id.ASK_COLOR)
				{
					byte code = data.get();
					if(code == LLStatus.COULEUR_JAUNE.codeInt)
						paquet.origine.ticket.set(LLStatus.COULEUR_JAUNE.etat, RobotColor.JAUNE);
					else if(code == LLStatus.COULEUR_VIOLET.codeInt)
						paquet.origine.ticket.set(LLStatus.COULEUR_VIOLET.etat, RobotColor.VIOLET);
					else
						paquet.origine.ticket.set(LLStatus.COULEUR_ROBOT_INCONNU.etat);
				}

				/**
				 * Capteurs
				 */
				else if(paquet.origine == Id.ODO_AND_SENSORS)
				{
					/**
					 * Récupération de la position et de l'orientation
					 */
					int xRobot = data.getInt();
					int yRobot = data.getInt();
					double orientationRobot = data.getFloat();
					double courbure = data.getFloat();
					int indexTrajectory = data.getInt();
					boolean enMarcheAvant = data.get() != 0;
					SensorsData sd = memory[indexMem]; 
					indexMem++;
					indexMem %= memory.length;
					
					for(CapteursRobot c : CapteursRobot.values())
					{
						sd.mesures[c.ordinal()] = data.getInt();
						int m = sd.mesures[c.ordinal()];
						if(capteursOn && m != CommProtocol.EtatCapteur.TROP_LOIN.ordinal())
							log.write("Capteur " + c.name() + " : " + (m < CommProtocol.EtatCapteur.values().length ? CommProtocol.EtatCapteur.values()[m] : m), Subject.CAPTEURS);
					}
					
					current.enMarcheAvant = enMarcheAvant;
					current.updateReel(xRobot, yRobot, orientationRobot, courbure);
					robot.setCurrentTrajectoryIndex(current, indexTrajectory);

					if(System.currentTimeMillis() - dernierAffichagePosition > 500)
					{
						dernierAffichagePosition = System.currentTimeMillis();
						log.write("Le robot est en " + current.getPosition() + ", orientation : " + orientationRobot + ", index : " + indexTrajectory, Subject.CAPTEURS);
					}

					if(capteursOn)
					{
						sd.cinematique = current;
						buffer.add(sd);
					}
//					else
//						log.write("Capteurs désactivés !", Subject.CAPTEURS);
				}

				/**
				 * Démarrage du match
				 */
				else if(paquet.origine == Id.WAIT_FOR_JUMPER)
				{
//					capteursOn = true;
					paquet.origine.ticket.set(CommProtocol.State.OK);
					log.write("DÉMARRAGE DU MATCH !", Subject.STATUS);
					robot.setDateDebutMatch();
				}
				
				else if(paquet.origine == Id.STOP ||
						paquet.origine == Id.DESTROY_POINTS ||
						paquet.origine == Id.ADD_POINTS)
					paquet.origine.ticket.set(CommProtocol.State.OK);
				
				else if(paquet.origine == Id.PING)
				{
					assert data.capacity() == 4 && data.getInt() == 0x00 : paquet;
					paquet.origine.ticket.set(CommProtocol.State.OK);
				}

				/*
				 * ACTIONNEURS
				 */				

				else if(paquet.origine.name().startsWith("ACTUATOR_"))
				{
					int code = (int) data.getInt();
					if(code == 0)
						paquet.origine.ticket.set(CommProtocol.State.OK);
					else
						paquet.origine.ticket.set(CommProtocol.State.KO, code);
				}				

				/**
				 * Fin du match, on coupe la série et on arrête ce thread
				 */
				else if(paquet.origine == Id.START_MATCH_CHRONO)
				{
					log.write("Fin du Match !", Subject.STATUS);
					container.interruptWithCodeError(ErrorCode.END_OF_MATCH);

					// On attend d'être arrêté
					while(true)
						Thread.sleep(1000);
				}

				/**
				 * Le robot est arrivé après un arrêt demandé par le haut niveau
				 */
				else if(paquet.origine == Id.FOLLOW_TRAJECTORY)
				{
					int code = data.getInt();
					if(code == 0)
						paquet.origine.ticket.set(CommProtocol.State.OK);
					else
						paquet.origine.ticket.set(CommProtocol.State.KO, CommProtocol.TrajEndMask.describe(code));
				}
				
				else
					assert false : "On a ignoré une réponse " + paquet.origine + " (taille : " + data.capacity() + ")";
				
				long duree = System.currentTimeMillis() - avant;
				if(duree >= 10)
					log.write("Durée de traitement de " + paquet.origine.name() + " : " + duree, duree >= 1000 ? Severity.CRITICAL : Severity.WARNING, Subject.COMM);
				
				duree = System.currentTimeMillis() - paquet.timestamp;
				if(duree >= 20)
					log.write("Latence de traitement de " + paquet.origine.name() + " : " + duree, duree >= 1000 ? Severity.CRITICAL : Severity.WARNING, Subject.COMM);
			}
		}
		catch(InterruptedException e)
		{
			log.write("Arrêt de " + Thread.currentThread().getName(), Subject.STATUS);
			Thread.currentThread().interrupt();
		}
		catch(Exception e)
		{
			log.write("Arrêt inattendu de " + Thread.currentThread().getName() + " : " + e, Subject.STATUS);
			e.printStackTrace();
			Thread.currentThread().interrupt();
		}
	}

}
