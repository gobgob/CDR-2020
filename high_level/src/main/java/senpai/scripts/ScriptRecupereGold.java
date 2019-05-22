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

import pfg.kraken.utils.XY;
import pfg.kraken.utils.XYO;
import pfg.kraken.utils.XY_RW;
import pfg.log.Log;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.capteurs.CapteursProcess;
import senpai.capteurs.CapteursRobot;
import senpai.comm.CommProtocol;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.ScriptException;
import senpai.exceptions.UnableToMoveException;
import senpai.robot.Robot;
import senpai.table.Table;
import senpai.table.TypeAtome;
import senpai.utils.Subject;

/**
 * Script de récupération du goldenium
 * @author pf
 *
 */

public class ScriptRecupereGold extends Script
{
	private XY_RW positionEntree = new XY_RW(-725,1665);
	private boolean done = false;
	
	public ScriptRecupereGold(Log log, Robot robot, Table table, CapteursProcess cp, OutgoingOrderBuffer out, boolean symetrie)
	{
		super(log, robot, table, cp, out);
		if(symetrie)
			positionEntree.setX(- positionEntree.getX());
	}

	@Override
	public String toString()
	{
		return getClass().getSimpleName();
	}

	@Override
	public XYO getPointEntree()
	{
		return new XYO(positionEntree, Math.PI / 2);
	}

	@Override
	protected void run() throws InterruptedException, UnableToMoveException, ActionneurException, ScriptException
	{		
		try {
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO, -23.7, 195., 2.);
			Object[] d = (Object[]) robot.execute(CommProtocol.Id.ACTUATOR_FIND_PUCK, Boolean.TRUE);
			if(d == null)
				throw new ActionneurException("No data after actuator find puck ?!", 0);
			double y = (double) d[0];
			int distance = (int) d[1];
			int code = (int) d[2];
			if(code == 0 || code == CommProtocol.ActionneurMask.NO_DETECTION.masque)
			{
				double distanceRobotMur = (distance + CapteursRobot.ToF_FOURCHE_DROITE.pos.getX()) * Math.cos(robot.getCinematique().orientationReelle - Math.PI/2);
				XY delta = new XY(0, - robot.getCinematique().getPosition().getY() + 2000 - 50 - distanceRobotMur); // palet à 5cm du bord
				log.write("Envoi d'une correction Y: " + delta.getY(), Subject.STATUS);
				robot.correctPosition(delta, 0);
				Thread.sleep(500); // update position LL
			}
			if(code != 0)
				throw new ActionneurException("No detection!", code);
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO, y, 182., 2.);
			robot.avanceTo(new XYO(positionEntree.getX(), 1665+95, Math.PI / 2));
			done = true; // le script n'est plus faisable
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, y, 182., 20., 1023., 300., 900.);
			robot.updateScore(20);
			robot.addToCargo(TypeAtome.Goldenium);
		}
		finally
		{
			// on ne recule que si on a avancé
			if(done)
				robot.avance(-100);
		}
	}
	
	@Override
	public boolean faisable()
	{
		return !done && !robot.isCargoFull(TypeAtome.Goldenium) && robot.isGoldeniumFree();
	}
	
	// Tolérance en position volontairement très grande : ce n'est pas elle qui sera limitante
	@Override
	public double getTolerancePosition()
	{
		return 100;
	}
	
	@Override
	public double getToleranceX()
	{
		return 15;
	}
	
	@Override
	public double getToleranceY()
	{
		return 50;
	}
}
