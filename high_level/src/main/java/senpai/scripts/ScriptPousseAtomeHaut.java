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

public class ScriptPousseAtomeHaut extends Script
{
	private XY_RW positionEntree = new XY_RW(860,1550); // point d'entrée du script
	private double angleEntree = 0; // angle d'entrée
	private XY_RW positionFin = new XY_RW(1294, 1449);
	private double angleFin = -0.3316;
	private boolean done = false;
	private AtomeParTerre at;
	
	public ScriptPousseAtomeHaut(Log log, Robot robot, Table table, CapteursProcess cp, OutgoingOrderBuffer out, boolean symetrie)
	{
		super(log, robot, table, cp, out);
		
		if(symetrie)
		{
			at = AtomeParTerre.HAUT_GAUCHE;
			positionEntree.setX(- positionEntree.getX());
			angleEntree = Math.PI - angleEntree;
		}
		else
			at = AtomeParTerre.HAUT_DROITE;

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
		try {
			robot.goTo(new XYO(positionFin, angleFin));
			robot.updateScore(6);
			// si tout s'est bien passé, alors le script n'est plus faisable
			table.setDone(at);
			done = true;
			robot.setScriptPousseAtomeHautFait();
		}
		catch (PathfindingException e) {
			log.write("Pathfinding exception lors du goTo", Subject.SCRIPT);
			throw new ScriptException();
		}
		finally
		{
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
		return !done && robot.isScriptPousseAtomeMilieuFait();
	}
	
}
