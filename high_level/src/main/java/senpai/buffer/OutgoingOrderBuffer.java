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

package senpai.buffer;

import static java.util.Comparator.comparing;
import static java.util.Comparator.naturalOrder;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.PriorityBlockingQueue;
import pfg.log.Log;
import senpai.comm.Order;
import senpai.comm.Ticket;
import senpai.robot.RobotColor;
import senpai.comm.CommProtocol;
import senpai.comm.CommProtocol.Channel;
import senpai.comm.CommProtocol.Id;
import senpai.utils.Severity;
import senpai.utils.Subject;
import pfg.kraken.struct.ItineraryPoint;
import pfg.kraken.struct.XY;

/**
 * Classe qui contient les ordres à envoyer à la série
 * Il y a trois priorité
 * - la plus haute, l'arrêt
 * - ensuite, la trajectoire courbe
 * - enfin, tout le reste
 * 
 * @author pf
 *
 */

public class OutgoingOrderBuffer
{
	protected Log log;

	public OutgoingOrderBuffer(Log log)
	{
		this.log = log;
	}
	
	private BlockingQueue<Order> buffer = new PriorityBlockingQueue<Order>(1000, comparing(Order::getPriority, naturalOrder()));

	public Order take() throws InterruptedException
	{
		boolean restart;
		Order o = null;
		do {
			if(o == null)
				o = buffer.take();
			
			// si on attend encore la réponse, on ignore cet ordre
			restart = !o.ordre.isSendPossible();
			if(restart)
			{
				// on récupère le prochain TOUT DE SUITE. Pas de prochain ? Tant pis.
				Order nextO = buffer.poll();
				addToBuffer(o); // on remet l'ordre fautif plus loin
				o = nextO;
			}
		} while(restart);
		o.ordre.orderSent();
		return o;
	}

	private void addToBuffer(Order o)
	{
		buffer.offer(o);
	}
	
	/**
	 * Ordre long de suivi de trajectoire
	 * 
	 * @param vitesseInitiale
	 * @param marcheAvant
	 * @return
	 */
	public Ticket followTrajectory()
	{
		log.write("On commence à suivre la trajectoire.", Subject.COMM);
		addToBuffer(new Order(Id.FOLLOW_TRAJECTORY));
		return Id.FOLLOW_TRAJECTORY.ticket;
	}

	/**
	 * Ajout d'une demande d'ordre de s'arrêter
	 */
	public Ticket immobilise()
	{
		log.write("Stop !", Severity.WARNING, Subject.COMM);
		addToBuffer(new Order(Id.STOP));
		return Id.STOP.ticket;
	}

	/**
	 * Corrige la position du bas niveau
	 */
	public void setPosition(XY pos, double orientation)
	{
		log.write("Set position en "+pos+", "+orientation, Subject.COMM);
		ByteBuffer data = ByteBuffer.allocate(12);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.putInt((int)pos.getX());
		data.putInt((int)pos.getY());
		data.putFloat((float) orientation);
		addToBuffer(new Order(data, Id.SET_POSITION));
	}

	/**
	 * Corrige la position du bas niveau
	 */
	public void correctPosition(XY deltaPos, double deltaOrientation)
	{
		ByteBuffer data = ByteBuffer.allocate(12);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.putInt((int)deltaPos.getX());
		data.putInt((int)deltaPos.getY());
		data.putFloat((float) deltaOrientation);
		addToBuffer(new Order(data, Id.EDIT_POSITION));
	}

	/**
	 * Demande à être notifié du début du match
	 */
	public Ticket waitForJumper()
	{
		addToBuffer(new Order(Id.WAIT_FOR_JUMPER));
		return Id.WAIT_FOR_JUMPER.ticket;
	}

	/**
	 * Demande à être notifié de la fin du match
	 */
	public Ticket startMatchChrono()
	{
		addToBuffer(new Order(Id.START_MATCH_CHRONO));
		return Id.START_MATCH_CHRONO.ticket;
	}

	/**
	 * Set la courbure
	 */
	public void setCurvature(double curvature)
	{
		ByteBuffer data = ByteBuffer.allocate(4);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.putFloat((float)curvature);
		addToBuffer(new Order(data, Id.SET_CURVATURE));
	}

	public void setScore(int score)
	{
		ByteBuffer data = ByteBuffer.allocate(4);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.putInt(score);
		addToBuffer(new Order(data, Id.SET_SCORE));		
	}
	
	/**
	 * Demande la couleur au bas niveau
	 */
	public Ticket demandeCouleur()
	{
		addToBuffer(new Order(Id.ASK_COLOR));
		return Id.ASK_COLOR.ticket;
	}
	
	/**
	 * Ping
	 */
	public Ticket ping()
	{
		addToBuffer(new Order(Id.PING));
		return Id.PING.ticket;
	}
	
	public void setNightLight(CommProtocol.NightLightMode mode)
	{
		ByteBuffer data = ByteBuffer.allocate(1);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.put((byte)mode.ordinal());
		addToBuffer(new Order(data, Id.SET_NIGHT_LIGHT));
	}
	
	public void setWarnings(boolean enable)
	{
		ByteBuffer data = ByteBuffer.allocate(1);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.put((byte)(enable ? 1 : 0));
		addToBuffer(new Order(data, Id.SET_WARNINGS));
	}
	
	public void enableSmoke(boolean enable)
	{
		ByteBuffer data = ByteBuffer.allocate(1);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.put((byte)(enable ? 1 : 0));
		addToBuffer(new Order(data, Id.SET_SMOKE_LEVEL));
	}
	
	public void enableParkingBreak(boolean enable)
	{
		ByteBuffer data = ByteBuffer.allocate(1);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.put((byte)(enable ? 1 : 0));
		addToBuffer(new Order(data, Id.ENABLE_PARKING_BREAK));
	}
	
	public void enableHighSpeedMode(boolean enable)
	{
		ByteBuffer data = ByteBuffer.allocate(1);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.put((byte)(enable ? 1 : 0));
		addToBuffer(new Order(data, Id.ENABLE_HIGH_SPEED_MODE));
	}
	
	public void displayColor(RobotColor c)
	{
		ByteBuffer data = ByteBuffer.allocate(1);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.put((byte)(c.ordinal()));
		addToBuffer(new Order(data, Id.DISPLAY_COLOR));
	}
	
	public void actuatorStop()
	{
		addToBuffer(new Order(Id.ACT_STOP));
	}
	
	public Ticket actuatorGetPosition()
	{
		addToBuffer(new Order(Id.ACT_GET_POSITION));
		return Id.ACT_GET_POSITION.ticket;
	}
	
	/**
	 * Démarre un stream
	 */
	public void startStream(Id stream)
	{
		changeStream(stream, Channel.INSCRIPTION);
	}
	
	/**
	 * Arrête un stream
	 */
	public void stopStream(Id stream)
	{
		changeStream(stream, Channel.DESINSCRIPTION);
	}
	
	private void changeStream(Id stream, Channel ch)
	{
		// on vérifie qu'on ne cherche pas à s'abonner alors qu'on est déjà abonné (idem avec désabonné)
		ByteBuffer data = ByteBuffer.allocate(1);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.put(ch.code);
		addToBuffer(new Order(data, stream));
		stream.changeStreamState(ch);
	}
	
	/**
	 * Détruit la trajectoire à partir de l'indice en paramètre (inclus)
	 * @param firstDestroyIndex
	 * @throws InterruptedException 
	 */
	public void destroyPointsTrajectoires(int firstDestroyIndex) throws InterruptedException
	{
		log.write("Destruction à partir de l'indice " + firstDestroyIndex, Subject.COMM);

		ByteBuffer data = ByteBuffer.allocate(4);
		data.order(ByteOrder.LITTLE_ENDIAN);
		data.putInt(firstDestroyIndex);
		addToBuffer(new Order(data, Id.DESTROY_POINTS));
		Id.DESTROY_POINTS.ticket.attendStatus(); // on attend d'avoir une confirmation !
	}
	
	/**
	 * Ajoute des points à la fin de la trajectoire
	 * @0 arc
	 * @throws InterruptedException 
	 */
	public void ajoutePointsTrajectoire(List<ItineraryPoint> points, boolean endOfTrajectory) throws InterruptedException
	{
		log.write("Ajout de " + points.size() + " points.", Subject.COMM);

		// on peut envoyer 11 arcs par trame au maximum
		int nbEnvoi = (points.size() + 10) / 11;
		int modulo = (points.size() + 10) % 11 + 1; // pour le dernier envoi
		for(int i = 0; i < nbEnvoi; i++)
		{
			int nbArc = 11;
			if(i == nbEnvoi - 1) // dernier envoi
				nbArc = modulo;
			
			ByteBuffer data = ByteBuffer.allocate(22 * nbArc);
			data.order(ByteOrder.LITTLE_ENDIAN);

			int k = 11 * i;
			for(int j = 0; j < nbArc; j++)
			{
				ItineraryPoint c = points.get(k);
				log.write("Point " + k + " : " + c, Subject.COMM);
				addPoint(data, c, endOfTrajectory && k == points.size() - 1);
				k++;
			}
			addToBuffer(new Order(data, Id.ADD_POINTS));
			Id.ADD_POINTS.ticket.attendStatus();
		}
	}
	
	private void addPoint(ByteBuffer data, ItineraryPoint point, boolean last)
	{
		data.putInt((int)point.x);
		data.putInt((int)point.y);
		data.putFloat((float) point.orientation);
		data.putFloat((float)point.curvature);
		if(point.goingForward)
			data.putFloat((float) (1000*point.maxSpeed));
		else
			data.putFloat((float) (-1000*point.maxSpeed));
		data.put((byte) (point.stop ? 1 : 0));
		data.put((byte) (last ? 1 : 0));
	}

	/**
	 * Renvoie une estimation de la latence de la comm, en μs
	 * @throws InterruptedException 
	 */
	public void checkLatence() throws InterruptedException
	{
		int nbEssais = 1000;
		long avant = System.currentTimeMillis();
		for(int i = 0; i < nbEssais; i++)
			ping().attendStatus();
		double latency = 1000. * (System.currentTimeMillis() - avant) / (2*nbEssais);
		// on divise par 2 car il s'agit d'un aller-retour
		log.write("Latence estimée : "+latency+" μs", latency >= 500 ? Severity.CRITICAL : (latency >= 300 ? Severity.WARNING : Severity.INFO), Subject.COMM);
	}
}
