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

package senpai.scripts;

import pfg.kraken.exceptions.PathfindingException;
import pfg.kraken.obstacles.CircularObstacle;
import pfg.kraken.obstacles.Obstacle;
import pfg.kraken.utils.XY;
import pfg.kraken.utils.XYO;
import pfg.kraken.utils.XY_RW;
import pfg.log.Log;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.capteurs.CapteursProcess;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.ScriptException;
import senpai.exceptions.UnableToMoveException;
import senpai.robot.Robot;
import senpai.table.AtomeParTerre;
import senpai.table.Table;
import senpai.utils.Subject;

/**
 * Script pour pousser un atome dans la zone de départ
 * @author pf
 *
 */

public class ScriptPousseAtomeMilieu extends Script
{
	private XY_RW positionEntree = new XY_RW(700,1250); // point d'entrée du script
	private double angleEntree = 0; // angle d'entrée
	private XY_RW positionDebut = new XY_RW(860,1250);
	private double angleDebut = 0; // angle d'entrée
	private XY_RW positionFin = new XY_RW(1279, 1347);
	private double angleFin = 0.4520;
	private boolean done = false;
	private AtomeParTerre at;
	private Obstacle obstacle;
	
	public ScriptPousseAtomeMilieu(Log log, Robot robot, Table table, CapteursProcess cp, OutgoingOrderBuffer out, boolean symetrie)
	{
		super(log, robot, table, cp, out);
		
		if(symetrie)
		{
			at = AtomeParTerre.MILIEU_GAUCHE;
			obstacle = new CircularObstacle(new XY(-1373, 1392), 38);
			positionEntree.setX(- positionEntree.getX());
			positionDebut.setX(- positionDebut.getX());
			positionFin.setX(- positionFin.getX());
			angleEntree = Math.PI - angleEntree;
			angleDebut = Math.PI - angleDebut;
			angleFin = Math.PI - angleFin;
		}
		else
		{
			at = AtomeParTerre.MILIEU_DROITE;
			obstacle = new CircularObstacle(new XY(1373, 1392), 38);
		}
	}

	@Override
	public String toString()
	{
		return getClass().getSimpleName();
	}

	@Override
	public XYO getPointEntree()
	{
		return new XYO(positionEntree, angleEntree);
	}

	@Override
	protected void run() throws InterruptedException, UnableToMoveException, ActionneurException, ScriptException
	{
		boolean hack_used = false;
		try {
			if (robot.isRobotDeploye()) {
				robot.HACK_setRobotNonDeploye();
				hack_used = true;
			}
			robot.avanceTo(new XYO(positionDebut, angleDebut));
			table.setDone(at);
			robot.goTo(new XYO(positionFin, angleFin));
			robot.updateScore(6);
			// si tout s'est bien passé, alors le script n'est plus faisable
			done = true;
			robot.setScriptPousseAtomeMilieuFait();
			table.addOtherObstacle(obstacle);
		}
		catch (PathfindingException e) {
			log.write("Pathfinding exception lors du goTo", Subject.SCRIPT);
			throw new ScriptException();
		}
		finally
		{
			if (hack_used) {
				robot.HACK_setRobotDeploye();
			}
			// dans tous les cas, on recule
			robot.avance(-200);
		}
	}

	@Override
	public double getToleranceAngle()
	{
		return 5;
	}
	
	@Override
	public double getTolerancePosition()
	{
		return 20;
	}
	
	@Override
	public boolean faisable()
	{
		return !done;
	}
	
}
