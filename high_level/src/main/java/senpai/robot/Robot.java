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

package senpai.robot;

import java.awt.Color;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
//import java.util.PriorityQueue;

import pfg.config.Config;
import pfg.graphic.printable.Layer;
import pfg.graphic.printable.Segment;
import pfg.kraken.Kraken;
import pfg.kraken.SearchParameters;
import pfg.kraken.display.Display;
import pfg.kraken.exceptions.PathfindingException;
import pfg.kraken.obstacles.CircularObstacle;
import pfg.kraken.obstacles.RectangularObstacle;
import pfg.kraken.struct.Kinematic;
import pfg.kraken.struct.ItineraryPoint;
import pfg.kraken.struct.XY;
import pfg.kraken.struct.XYO;
import pfg.kraken.struct.XY_RW;
import pfg.log.Log;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.comm.CommProtocol;
import senpai.comm.DataTicket;
import senpai.comm.Ticket;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.UnableToMoveException;
import senpai.scripts.GameState;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Severity;
import senpai.utils.Subject;

/**
 * Les méthodes que peut faire le robot
 * 
 * @author pf
 */

public class Robot
{
	public enum State
	{
		STANDBY, // le robot est à l'arrêt
		READY_TO_GO, // une trajectoire a été envoyée
		STOPPING, // une commande de stop a été envoyée
		MOVING; // le robot se déplace
	}
	
/*	private volatile long lidarCorrectionTimeOut = 0;
	private volatile boolean needLidarCorrection = false;
	private volatile long lastCorrectionDate = 0;*/
	protected volatile boolean symetrie;
	protected Log log;
	private double defaultSpeed;//, maxSpeedInEnemy;
	protected Kraken krakenDeploye;
	protected Kraken krakenRange;
	private RectangularObstacle obstacle;
	private GameState gamestate;
	private List<ItineraryPoint> path = null;
	private volatile long dateDebutMatch = Long.MAX_VALUE, dateFinMatch = Long.MAX_VALUE;
	private RobotColor c = null;
	private boolean deploye = false;
	private Kinematic cinematique;
	
	private boolean jumperOK = false;
	private volatile State etat = State.STANDBY;
	private final boolean simuleLL;
	private final boolean printTrace;
	private OutgoingOrderBuffer out;
	private Display buffer;
	private RobotPrintable printable = null;
	private boolean graphicPath;
	private volatile boolean cinematiqueInitialised = false;
	private int currentIndexTrajectory = 0;
	private int score;
//	private CircularObstacle[] lidarObs = new CircularObstacle[100]; // pas plus de cent obstacles
//	private volatile boolean enableLidar;
	
	public Robot(Log log, GameState gamestate, OutgoingOrderBuffer out, Config config, Display buffer, Kraken[] krakens, /*DynamicPath dpath,*/ /*KnownPathManager known,*/ RectangularObstacle obstacle)
	{
		this.gamestate = gamestate;
		this.log = log;
		this.out = out;
		this.buffer = buffer;
		this.krakenDeploye = krakens[0];
		this.krakenRange = krakens[1];
//		this.known = known;
		this.obstacle = obstacle;

		jumperOK = config.getBoolean(ConfigInfoSenpai.DISABLE_JUMPER);
		defaultSpeed = config.getDouble(ConfigInfoSenpai.DEFAULT_MAX_SPEED);
//		maxSpeedInEnemy = config.getDouble(ConfigInfoSenpai.MAX_SPEED_IN_ENEMY);
		graphicPath = config.getBoolean(ConfigInfoSenpai.GRAPHIC_PATH);
//		enableLidar = config.getBoolean(ConfigInfoSenpai.ENABLE_LIDAR);
		
		// On ajoute une fois pour toute l'image du robot
		if(config.getBoolean(ConfigInfoSenpai.GRAPHIC_ROBOT_AND_SENSORS))
		{
			printable = new RobotPrintable(config);
			buffer.addPrintable(printable, Color.BLACK, Layer.MIDDLE.layer);
		}
		
//		enableLoadPath = config.getBoolean(ConfigInfoSenpai.ENABLE_KNOWN_PATHS);
		printTrace = config.getBoolean(ConfigInfoSenpai.GRAPHIC_TRACE_ROBOT);
		cinematique = new Kinematic(new XYO(
				config.getDouble(ConfigInfoSenpai.INITIAL_X),
				config.getDouble(ConfigInfoSenpai.INITIAL_Y),
				config.getDouble(ConfigInfoSenpai.INITIAL_O)));
		cinematique.enMarcheAvant = true;

		simuleLL = config.getBoolean(ConfigInfoSenpai.SIMULE_COMM);
		score = 0;
		updateScore();
		out.setCurvature(0);
	}

	
	public void setEnMarcheAvance(boolean enMarcheAvant)
	{
		cinematique.enMarcheAvant = enMarcheAvant;
	}
	
	@Override
	public String toString()
	{
		return cinematique.toString();
	}

	private XY_RW oldPosition = new XY_RW();
	
	public void setCinematique(Kinematic cinematique)
	{
		this.cinematique.getPosition().copy(oldPosition);
		cinematique.copy(this.cinematique);
		obstacle.update(cinematique.getPosition(), cinematique.orientationReelle);

		/*
		 * On vient juste de récupérer la position initiale
		 */
		if(printTrace)
			synchronized(buffer)
			{
				// affichage
				if(oldPosition.distanceFast(cinematique.getPosition()) < 100)
					buffer.addPrintable(new Segment(oldPosition, cinematique.getPosition().clone()), Color.RED, Layer.MIDDLE.layer);
			}
	}
	
	/*
	 * ACTIONNEURS
	 */

	/**
	 * Rend bloquant l'appel d'une méthode
	 * 
	 * @param m
	 * @throws InterruptedException
	 * @throws ActionneurException
	 */
	protected Object bloque(String nom, Object... param) throws InterruptedException, ActionneurException
	{
		if(param == null || param.length == 0)
			log.write("Appel à " + nom, Subject.SCRIPT);
		else
		{
			String s = "";
			for(Object o : param)
			{
				if(!s.isEmpty())
					s += ", ";
				s += o;
			}
			log.write("Appel à " + nom + " (param = " + s + ")", Subject.SCRIPT);
		}

		if(simuleLL)
			return null;

		Ticket t = null;
		Class<?>[] paramClasses = null;
		if(param.length > 0)
		{
			paramClasses = new Class[param.length];
			for(int i = 0; i < param.length; i++)
				paramClasses[i] = param[i].getClass();
		}
		long avant = System.currentTimeMillis();
		try
		{
			t = (Ticket) OutgoingOrderBuffer.class.getMethod(nom, paramClasses).invoke(out, param.length == 0 ? null : param);
		}
		catch(IllegalAccessException | IllegalArgumentException | InvocationTargetException | NoSuchMethodException | SecurityException e)
		{
			e.printStackTrace();
			throw new ActionneurException("Méthode inconnue : " + nom, -1);
		}
		DataTicket dt;
		try {
			 dt = t.attendStatus();
		} catch(InterruptedException e)
		{
			log.write("Interruption de l'actionneur: "+e, Severity.CRITICAL, Subject.SCRIPT);
			throw e;
		}
		log.write("Temps d'exécution de " + nom + " : " + (System.currentTimeMillis() - avant), Subject.SCRIPT);

		if(dt.status == CommProtocol.State.KO)
			throw new ActionneurException("Problème pour l'actionneur " + nom+" : "+CommProtocol.ActionneurMask.describe((int)dt.data), (int)dt.data);

		return dt.data;
	}
	
	public void avanceTo(XYO xyo) throws InterruptedException, UnableToMoveException
	{
		avanceTo(xyo, defaultSpeed);
	}

	public void avanceTo(XYO xyo, double vitesseMax) throws InterruptedException, UnableToMoveException
	{
		double distance = xyo.position.distance(cinematique.getPosition());
		if(xyo.position.minusNewVector(cinematique.getPosition()).dot(new XY(100, cinematique.orientationReelle, true)) < 0)
			distance = -distance;
		
		if(distance >= 0)
			log.write("On avance de "+distance+" mm", Subject.TRAJECTORY);
		else
			log.write("On recule de "+(-distance)+" mm", Subject.TRAJECTORY);
		
		LinkedList<ItineraryPoint> ch = new LinkedList<ItineraryPoint>();
		double cos = Math.cos(xyo.orientation);
		double sin = Math.sin(xyo.orientation);
		int nbPoint = (int) Math.round(Math.abs(distance) / 20);
		boolean marcheAvant = distance > 0;
		if(nbPoint == 0)
		{
			// Le point est vraiment tout proche
			ch.add(new ItineraryPoint(xyo.position.getX(), xyo.position.getY(), xyo.orientation, 0, marcheAvant, vitesseMax, true));
		}
		else
		{
			double deltaX = 20 * cos;
			double deltaY = 20 * sin;
			if(distance < 0)
			{
				deltaX = -deltaX;
				deltaY = -deltaY;
			}

			for(int i = 0; i < nbPoint; i++)
				ch.addFirst(new ItineraryPoint(xyo.position.getX() - i * deltaX, xyo.position.getY() - i * deltaY, xyo.orientation, 0, marcheAvant, vitesseMax, i == 0));
		}

		if(!simuleLL)
		{
			out.destroyPointsTrajectoires(0);
			out.ajoutePointsTrajectoire(ch, true);
	
			path = ch;
			setReady();
	
			DataTicket dt = followTrajectory();
			if(dt.data != null)
				throw new UnableToMoveException(dt.data.toString());
		}
		else
			cinematique.updateReel(xyo.position.getX(), xyo.position.getY(), cinematique.orientationReelle, 0);
	}
	
	public void avance(double distance) throws InterruptedException, UnableToMoveException
	{
		avance(distance, defaultSpeed);
	}

	public void avance(double distance, double vitesseMax) throws InterruptedException, UnableToMoveException
	{
		if(distance >= 0)
			log.write("On avance de "+distance+" mm", Subject.TRAJECTORY);
		else
			log.write("On recule de "+(-distance)+" mm", Subject.TRAJECTORY);
		
		LinkedList<ItineraryPoint> ch = new LinkedList<ItineraryPoint>();
		double cos = Math.cos(cinematique.orientationReelle);
		double sin = Math.sin(cinematique.orientationReelle);
		int nbPoint = (int) Math.round(Math.abs(distance) / 20);
		double xFinal = cinematique.getPosition().getX() + distance * cos;
		double yFinal = cinematique.getPosition().getY() + distance * sin;
		boolean marcheAvant = distance > 0;
		if(nbPoint == 0)
		{
			// Le point est vraiment tout proche
			ch.add(new ItineraryPoint(xFinal, yFinal, cinematique.orientationReelle, 0, marcheAvant, vitesseMax, true));
		}
		else
		{
			double deltaX = 20 * cos;
			double deltaY = 20 * sin;
			if(distance < 0)
			{
				deltaX = -deltaX;
				deltaY = -deltaY;
			}

			for(int i = 0; i < nbPoint; i++)
				ch.addFirst(new ItineraryPoint(xFinal - i * deltaX, yFinal - i * deltaY, cinematique.orientationReelle, 0, marcheAvant, vitesseMax, i == 0));
//			System.out.println("Trajectoire : "+ch);
		}

		if(!simuleLL)
		{
			out.destroyPointsTrajectoires(0);
			out.ajoutePointsTrajectoire(ch, true);
	
			path = ch;
			setReady();
	
			DataTicket dt = followTrajectory();
			if(dt.data != null)
				throw new UnableToMoveException(dt.data.toString());
		}
		else
			cinematique.updateReel(xFinal, yFinal, cinematique.orientationReelle, 0);
	}
	
	public Object execute(CommProtocol.Id ordre, Object... param) throws InterruptedException, ActionneurException
	{
		// TODO
//		deploye = ordre != CommProtocol.Id.ACTUATOR_GO_HOME;
		int nbEssaiMax = 2;
		boolean retry;
		do {
			retry = false;
			try {
				return bloque(ordre.getMethodName(), param);
			}
			catch(ActionneurException e)
			{
				int code = e.code;
				
				if(code == CommProtocol.ActionneurMask.AX12_ERR.masque)
				{
					log.write("AX12_ERR ignoré ! "+e, Subject.SCRIPT);
					break;
				}
				
				nbEssaiMax--;
				if(nbEssaiMax >= 0 && e.code == CommProtocol.ActionneurMask.MOVE_TIMED_OUT.masque)
					retry = true;
				else
					throw e;
			}
		} while(retry);
		
		assert false;
		return null;
	}

	public void updateColorAndSendPosition(RobotColor c) throws InterruptedException
	{
		this.c = c;
		assert cinematique != null;
		symetrie = c.symmetry;

		// avec la correction, la position est déjà à jour et envoyée
		
		// on applique la symétrie à la position initiale
		if(symetrie)
			setCinematique(new Kinematic(-cinematique.getPosition().getX(),
					cinematique.getPosition().getY(),
					Math.PI - cinematique.orientationReelle,
					cinematique.enMarcheAvant,
					cinematique.courbureReelle,
					false));

		// on envoie la position au LL
		out.setPosition(cinematique.getPosition(), cinematique.orientationReelle);
		Thread.sleep(100);
		cinematiqueInitialised = true;
		out.displayColor(c);
	}

	/*
	 * On a besoin d'initialiser à part car elle est utilisée pour centre l'affichage graphique
	 */
	public void initPositionObject(Kinematic c)
	{
		cinematique.copy(c);
		cinematique = c;
		
		// on active le printable
		if(printable != null)
			printable.initPositionObject(cinematique);
	}

	XY_RW tmp = new XY_RW();
/*	public List<ItineraryPoint> slowDownTrajectory(List<ItineraryPoint> path)
	{
		List<ItineraryPoint> out = new ArrayList<ItineraryPoint>();
		log.write("On ralentit la trajectoire si nécessaire", Subject.TRAJECTORY);
		int size = path.size();
		assert lidarObs != null;
		for(ItineraryPoint ip : path)
		{
			tmp.setX(ip.x);
			tmp.setY(ip.y);
			boolean changed = false;
			for(CircularObstacle s: lidarObs)
			{
				if(s != null && s.isInObstacle(tmp))
				{
					double maxSpeed = Math.min(ip.maxSpeed, maxSpeedInEnemy);
					ItineraryPoint newIp = new ItineraryPoint(ip.x, ip.y, ip.orientation, ip.curvature, ip.goingForward, maxSpeed, ip.stop);
					out.add(newIp);
					changed = true;
					break;
				}
			}
			if(!changed)
				out.add(ip);
		}
		assert out.size() == size : out.size()+" "+size;
		return out;	
	}*/
	
	public synchronized void setReady()
	{
		assert etat == State.STANDBY;
		etat = State.READY_TO_GO;
		notifyAll();
	}
	
	public DataTicket goTo(XYO destination, boolean reverse) throws PathfindingException, InterruptedException, UnableToMoveException
	{
		if(reverse)
			return goTo(new SearchParameters(destination, cinematique.getXYO()), true);
		else
			return goTo(new SearchParameters(cinematique.getXYO(), destination), false);
	}

	
	public DataTicket goTo(XYO destination) throws PathfindingException, InterruptedException, UnableToMoveException
	{
		return goTo(destination, false);
	}


	private DataTicket goTo(SearchParameters sp, boolean reverse) throws PathfindingException, InterruptedException, UnableToMoveException
	{
		if(graphicPath)
		{
			RectangularObstacle arrival = obstacle.clone();
			arrival.update(sp.arrival.getPosition(), sp.arrival.orientationReelle);
			buffer.addPrintable(arrival, Color.BLUE, Layer.FOREGROUND.layer);
		}
		long avant = System.currentTimeMillis();
		Kraken k;
		if(deploye)
		{
			log.write("On utilise le Kraken déployé", Subject.TRAJECTORY);
			k = krakenDeploye;
		}
		else
		{
			log.write("On utilise le Kraken non-déployé", Subject.TRAJECTORY);
			k = krakenRange;
		}
		k.initializeNewSearch(sp);
		log.write("Durée d'initialisation de Kraken : "+(System.currentTimeMillis() - avant), Subject.TRAJECTORY);

		log.write("On cherche un chemin", Subject.TRAJECTORY);
		avant = System.currentTimeMillis();
		path = k.search();
		if(reverse)
		{
			List<ItineraryPoint> tmp = new ArrayList<ItineraryPoint>();
			for(int i = path.size()-2; i >= 0; i--)
			{
				ItineraryPoint ip = path.get(i);
				tmp.add(new ItineraryPoint(ip.x, ip.y, ip.orientation, ip.curvature, !ip.goingForward, ip.maxSpeed, ip.stop));
			}
			tmp.add(new ItineraryPoint(sp.start.getPosition().getX(), sp.start.getPosition().getY(), sp.start.orientationReelle, tmp.get(tmp.size()-1).curvature,
					tmp.get(tmp.size()-1).goingForward, tmp.get(tmp.size()-1).maxSpeed, true));
			path = tmp;
		}
		if(graphicPath)
			for(ItineraryPoint ip: path)
				buffer.addPrintable(ip, Color.BLACK, Layer.FOREGROUND.layer);
		System.out.println(path.get(0));
		log.write("Durée de la recherche : "+(System.currentTimeMillis() - avant), Subject.TRAJECTORY);
		
//		path = slowDownTrajectory(path);
		
		if(!simuleLL)
		{
			log.write("On envoie la trajectoire", Subject.TRAJECTORY);
			out.destroyPointsTrajectoires(0);
			out.ajoutePointsTrajectoire(path, true);
		}
		setReady();

		DataTicket out = null;
		
		out = followTrajectory();
		
//		if(enableLidar)
//			requestLidarCorrection();
		
		if(!simuleLL && out.data != null)
			throw new UnableToMoveException(out.data.toString());

		return out;
	}
	
/*	public void requestLidarCorrection()
	{
		log.write("Lidar request!", Subject.STATUS);
		needLidarCorrection = true;
	}*/
	
	private DataTicket followTrajectory() throws InterruptedException
	{
		// non, marche pas avec avancer et reculer
		assert etat == State.READY_TO_GO || etat == State.STANDBY;

		log.write("Attente de la trajectoire...", Subject.TRAJECTORY);

		assert etat == State.READY_TO_GO;
		synchronized(this)
		{
			while(etat == State.STANDBY)
				wait();
		}
		
/*		if(enableLidar)
		{
			log.write("Attente de la correction lidar...", Subject.TRAJECTORY);
			while(enableLidar && (!isLidarTimeout() || needLidarCorrection))
				Thread.sleep(5);
		}*/
		
		if(!jumperOK)
		{
			log.write("La trajectoire est prête : attente du jumper !", Subject.TRAJECTORY);
			out.waitForJumper().attendStatus();
			out.startMatchChrono();
			jumperOK = true;
		}
		
		log.write("On commence à suivre la trajectoire de "+path.size()+" points", Subject.TRAJECTORY);

		assert etat == State.READY_TO_GO;
		setMoving();
		
		DataTicket dt;
		
		if(!simuleLL)
		{
			Ticket t = out.followTrajectory();
		
			dt = t.attendStatus();
//			assert etat != State.MOVING : etat;
//			while(etat == State.MOVING)
//				wait();
			if(dt.data == null)
				log.write("Le robot a fini correctement la trajectoire. Position finale : "+cinematique.getXYO(), Subject.TRAJECTORY);
			else
				log.write("Le robot s'est arrêté suite à un problème : "+dt.data, Severity.CRITICAL, Subject.TRAJECTORY);
		}
		else
		{
			dt = new DataTicket(path, CommProtocol.State.OK);
			cinematique.updateReel(path.get(path.size()-1).x, path.get(path.size()-1).y, path.get(path.size()-1).orientation, 0);
		}
		
		path = null;
		etat = State.STANDBY;
		return dt;
	}

/*	public boolean isLidarTimeout()
	{
		return System.currentTimeMillis() > lidarCorrectionTimeOut;
	}*/
	
	public synchronized void setStopping()
	{
		etat = State.STOPPING;
		notifyAll();
	}

	
	private synchronized void setMoving()
	{
		etat = State.MOVING;
		notifyAll();
	}

	public synchronized boolean isStandby()
	{
		return etat == State.STANDBY;
	}

/*	public synchronized void setDegrade()
	{
		if(!modeDegrade)
		{
			log.write("Le robot entre en mode dégradé !", Severity.WARNING, Subject.STATUS);
			kraken.endAutoReplanning();
			modeDegrade = true;
			notifyAll();
		}
	}
	
	public boolean isDegrade() {
		return modeDegrade;
	}*/

	public boolean needCollisionCheck()
	{
		return etat == State.MOVING;
	}

	public List<ItineraryPoint> getPath()
	{
		assert etat == State.READY_TO_GO || etat == State.MOVING;
		return path;
	}

	public boolean isProcheRobot(XY positionVue, int distance)
	{
		return obstacle.isProcheObstacle(positionVue, distance);
	}

	public void setCurrentTrajectoryIndex(Kinematic current, int indexTrajectory)
	{	
		currentIndexTrajectory = indexTrajectory;
//		chemin.setCurrentTrajectoryIndex(indexTrajectory);
		if(cinematiqueInitialised)
			setCinematique(current);
	}

	public void updateScore()
	{
		updateScore(0);
	}
	
	public void updateScore(int increment)
	{
		score += increment;
		out.setScore(score);
	}

	public int getIndexTrajectory()
	{
		return currentIndexTrajectory;
	}

	public void printTemps()
	{
		log.write("Temps depuis le début du match : "+(System.currentTimeMillis() - dateDebutMatch), Subject.STATUS);
	}
	
	public long getTempsRestant()
	{
		return dateFinMatch - System.currentTimeMillis();
	}
	
	public void setDateDebutMatch()
	{
		dateDebutMatch = System.currentTimeMillis();
		dateFinMatch = dateDebutMatch + 100000;
	}

	public void correctPosition(XY position, double orientation)
	{
		cinematique.updateReel(cinematique.getPosition().getX() + position.getX(),
				cinematique.getPosition().getY() + position.getY(),
				cinematique.orientationReelle + orientation, cinematique.courbureReelle);
		out.correctPosition(position, orientation);
	}

/*	public void setLidarObs(CircularObstacle obs, int id)
	{
		lidarObs[id] = obs;
	}
	
	public void clearLidarObs()
	{
		for(int i = 0; i < lidarObs.length; i++)
			lidarObs[i] = null;
	}*/

	public void initActionneurs() throws ActionneurException, InterruptedException
	{
		rangeSiPossible();
	}
	
	public void HACK_setRobotNonDeploye()
	{
		deploye = false;
	}

	public void HACK_setRobotDeploye()
	{
		deploye = true;
	}

	public boolean isRobotDeploye()
	{
		return deploye;
	}

	public void rangeSiPossible() throws InterruptedException, ActionneurException
	{
		if(deploye)
		{
			// TODO
		}
	}
	

	public RobotColor getColor()
	{
		return c;
	}
	
	public boolean isMatchStarted()
	{
		return dateDebutMatch != Long.MAX_VALUE;
	}
	
	public boolean isMatchStopped()
	{
		return getTempsRestant() < 0;
	}
	
/*	public boolean isLidarCorrectionAllowed()
	{
		return System.currentTimeMillis() - lastCorrectionDate > 2000; // pas de correction lidar moins de 2s apprès une correction par capteurs
	}
	
	public void startLidarCorrection()
	{
		lidarCorrectionTimeOut = System.currentTimeMillis() + 2000;
	}
	
	public void disableLidar()
	{
		log.write("On n'utilise plus le lidar", Subject.STATUS);
		enableLidar = false;
	}
	
	public void stopLidarCorrection()
	{
		lidarCorrectionTimeOut = 0;
	}
	
	public boolean needLidarCorrection()
	{
		if(needLidarCorrection)
		{
			needLidarCorrection = false;
			return true;
		}
		return false;
	}

	public String printLidarStatus()
	{
		return "lidarEnable: "+enableLidar+
				", needLidarCorrection: "+needLidarCorrection+
				", lastCorrectionCapteurs: "+lastCorrectionDate+
				", lidarCorrectionTimeOut: "+lidarCorrectionTimeOut+
				", delta lastCorrectionCapteurs: "+(System.currentTimeMillis() - lastCorrectionDate)+
				", delta lidarCorrectionTimeOut: "+(System.currentTimeMillis() - lidarCorrectionTimeOut);
	}

	public void setLastCorrectionDate()
	{
		lastCorrectionDate = System.currentTimeMillis();
	}*/

	public Kinematic getCinematique()
	{
		return cinematique;
	}
	
}
