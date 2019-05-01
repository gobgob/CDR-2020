package senpai;

import pfg.config.Config;
import pfg.kraken.exceptions.PathfindingException;
import pfg.kraken.utils.XYO;
import pfg.log.Log;
import senpai.Senpai.ErrorCode;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.comm.CommProtocol;
import senpai.comm.DataTicket;
import senpai.comm.Ticket;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.ScriptException;
import senpai.exceptions.UnableToMoveException;
import senpai.robot.Robot;
import senpai.robot.RobotColor;
import senpai.scripts.Script;
import senpai.scripts.ScriptManager;
import senpai.table.Table;
import senpai.threads.comm.ThreadCommProcess;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Severity;
import senpai.utils.Subject;

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

/**
 * Match !
 * @author pf
 *
 */

public class Match
{
	private static Senpai senpai;
	private OutgoingOrderBuffer ll;
	private Robot robot;
	private Table table;
	private ScriptManager scripts;
	private Log log;
	private Config config;

	/**
	 * Gestion des paramètres et de la fermeture
	 * @param args
	 */
	public static void main(String[] args)
	{
		ErrorCode error = ErrorCode.NO_ERROR;
		try {
			if(args.length == 1)
				new Match().exec(args[0]);
			else
			{
				throw new Exception("Paramètre obligatoire : nom du fichier de configuration");
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
			error = ErrorCode.EXCEPTION;
			error.setException(e);
		}
		finally
		{
			try
			{
				if(senpai != null)
					senpai.destructor(error);
			}
			catch(InterruptedException e)
			{
				e.printStackTrace();
			}
		}
	}
	
	public void exec(String configFile) throws InterruptedException
	{
		/**
		 * Initialisation
		 */
		
		String configfile = configFile;
		
		senpai = new Senpai();
		senpai.initialize(configfile, "default", "graphic");
		config = senpai.getService(Config.class);
		ll = senpai.getService(OutgoingOrderBuffer.class);
		robot = senpai.getService(Robot.class);
		table = senpai.getService(Table.class);
		scripts = senpai.getService(ScriptManager.class);
		log = senpai.getService(Log.class);
		
		RobotColor couleur;

		/*
		 * Attente de la couleur
		 */

		if(config.getBoolean(ConfigInfoSenpai.DISABLE_JUMPER))
		{
			couleur = RobotColor.VIOLET;
			robot.setDateDebutMatch();
		}
		else
		{
			DataTicket etat;
			do
			{
				// Demande la couleur toute les 100ms et s'arrête dès qu'elle est connue
				Ticket tc = ll.demandeCouleur();
				etat = tc.attendStatus();
				Thread.sleep(100);
			} while(etat.status != CommProtocol.State.OK);
			couleur = (RobotColor) etat.data;
		}

		/*
		 * Allumage des capteurs
		 */
		senpai.getService(ThreadCommProcess.class).capteursOn = true;
		

		
		Script accelerateur = scripts.getScriptAccelerateur();
		Script recupereGold = scripts.getScriptRecupereGold();
		Script recupereDistrib = scripts.getScriptRecupereDistrib();
		Script deposeBalance = scripts.getScriptDeposeBalance();
		
		/*
		 * Recalage initial
		 */
/*		boolean byLL;
		ScriptRecalageInitial rec = scripts.getScriptRecalageInitial();
		try {
			RobotColor couleurRecalage;
			rec.execute();
			XYO initialCorrection = rec.getCorrection();
			double deltaX = Math.round(initialCorrection.position.getX())/10.;
			double deltaY = Math.round(initialCorrection.position.getY())/10.;
			double orientation = initialCorrection.orientation * 180. / Math.PI;
			log.write("Je suis "+Math.abs(deltaX)+"cm sur la "+(deltaX < 0 ? "droite" : "gauche"), Subject.STATUS);
			log.write("Je suis "+Math.abs(deltaY)+"cm vers l'"+(deltaY < 0 ? "avant" : "arrière"), Subject.STATUS);
			log.write("Je suis orienté vers la "+(orientation < 0 ? "droite" : "gauche")+" de "+Math.abs(orientation)+"°", Subject.STATUS);

			if(robot.getCinematique().getPosition().getX() > 0)
				couleurRecalage = RobotColor.VIOLET;
			else
				couleurRecalage = RobotColor.JAUNE;
			if(couleur != couleurRecalage)
			{
				log.write("Conflit de couleur ! LL : "+couleur+", recalage : "+couleurRecalage, Severity.CRITICAL, Subject.STATUS);
				couleur = couleurRecalage;
			}
			byLL = false;
		} catch (ScriptException e) {
			log.write("Aucun recalage possible, on prend la couleur du bas niveau", Severity.WARNING, Subject.STATUS);
			byLL = true;
		}

		log.write("Couleur utilisée : "+couleur, Subject.STATUS);
		robot.updateColorAndSendPosition(couleur, byLL);*/
		scripts.setCouleur(couleur);

		try {
			robot.initActionneurs();
		} catch (ActionneurException e1) {
			log.write("Erreur lors de l'initialisation du bras : "+e1, Subject.STATUS);
		}

		while(true)
		{
			try
			{
				doScript(accelerateur, 3, true);
			}
			catch(PathfindingException | UnableToMoveException | ScriptException e)
			{
				log.write("Erreur : "+e, Subject.SCRIPT);
			}
			
			try
			{
				doScript(recupereDistrib, 3, true);
			}
			catch(PathfindingException | UnableToMoveException | ScriptException e)
			{
				log.write("Erreur : "+e, Subject.SCRIPT);
			}
			
			try
			{
				doScript(deposeBalance, 3, true);
			}
			catch(PathfindingException | UnableToMoveException | ScriptException e)
			{
				log.write("Erreur : "+e, Subject.SCRIPT);
			}
			
			try
			{
				doScript(recupereGold, 3, true);
			}
			catch(PathfindingException | UnableToMoveException | ScriptException e)
			{
				log.write("Erreur : "+e, Subject.SCRIPT);
			}
			
			try
			{
				doScript(deposeBalance, 3, true);
			}
			catch(PathfindingException | UnableToMoveException | ScriptException e)
			{
				log.write("Erreur : "+e, Subject.SCRIPT);
			}
			
		}
		

	}
	
	private void doScript(Script s, int nbEssaiChemin, boolean checkFin) throws PathfindingException, InterruptedException, UnableToMoveException, ScriptException
	{
		if(Thread.currentThread().isInterrupted())
			throw new InterruptedException();

		log.write("Essai du script "+s, Subject.SCRIPT);
		if(!s.faisable())
			throw new ScriptException("Script n'est pas faisable !");
		
		XYO pointEntree = s.getPointEntree();
		log.write("Point d'entrée du script "+pointEntree, Subject.SCRIPT);
		
		boolean restart;
		do {
			try {
				restart = false;
				robot.goTo(pointEntree);
				if(Thread.currentThread().isInterrupted())
					throw new InterruptedException();
			}
			catch(UnableToMoveException e)
			{
				restart = true;
				nbEssaiChemin--;
			}
		} while(restart && nbEssaiChemin > 0);
		
		if(checkFin)
		{
			double toleranceAngle = 5; // en degré
			double tolerancePosition = 40; // en mm
			tolerancePosition *= tolerancePosition;
			if(Math.abs(XYO.angleDifference(robot.getCinematique().orientationReelle, pointEntree.orientation)) > toleranceAngle*Math.PI/180
					|| robot.getCinematique().getPosition().squaredDistance(pointEntree.position) > tolerancePosition)
				// on retente
				robot.goTo(pointEntree);
	
			if(Math.abs(XYO.angleDifference(robot.getCinematique().orientationReelle, pointEntree.orientation)) > toleranceAngle*Math.PI/180
					|| robot.getCinematique().getPosition().squaredDistance(pointEntree.position) > tolerancePosition)
				throw new ScriptException("On n'a pas réussi à se positionner une précision suffisante.");
		}
		
		if(!restart)
			s.execute();
		else
			log.write("On annule l'exécution du script "+s, Subject.SCRIPT);

		if(Thread.currentThread().isInterrupted())
			throw new InterruptedException();
	}
}
