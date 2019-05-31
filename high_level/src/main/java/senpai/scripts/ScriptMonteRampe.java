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
import senpai.capteurs.CapteursCorrection;
import senpai.capteurs.CapteursProcess;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.ScriptException;
import senpai.exceptions.UnableToMoveException;
import senpai.robot.Robot;
import senpai.table.AtomeParTerre;
import senpai.table.Table;
import senpai.table.TypeAtome;
import senpai.comm.CommProtocol;
import senpai.utils.Subject;

/**
 * Script de la montée de la rampe
 * @author pf
 *
 */

public class ScriptMonteRampe extends Script
{
	private XY_RW positionEntree = new XY_RW(1275,335); // point d'entrée du script
	private double angleEntree = -Math.PI / 2; // angle d'entrée du script
	private XY_RW positionGoTo = new XY_RW(1000,200); // point d'entrée de la rampe
	private double angleGoTo = Math.PI; // angle d'entrée de la rampe
	private XY_RW positionAvance = new XY_RW(350,200); // jusqu'où le robot doit-il avancer
	private double angleAvance = Math.PI; // angle d'arrivée
	private boolean done = false;
	private AtomeParTerre at;
	
	public ScriptMonteRampe(Log log, Robot robot, Table table, CapteursProcess cp, OutgoingOrderBuffer out, boolean symetrie)
	{
		super(log, robot, table, cp, out);
		if(symetrie)
		{
			at = AtomeParTerre.PENTE_DROITE;
			positionEntree.setX(- positionEntree.getX());
			angleEntree = Math.PI - angleEntree;
			positionGoTo.setX(- positionGoTo.getX());
			angleGoTo = Math.PI - angleGoTo;
			positionAvance.setX(- positionAvance.getX());
			angleAvance = Math.PI - angleAvance;
		}
		else
			at = AtomeParTerre.PENTE_GAUCHE;

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
	public XYO correctOdo() throws InterruptedException
	{
		return cp.doStaticCorrection(500, CapteursCorrection.AVANT);
	}

	@Override
	protected void run() throws InterruptedException, UnableToMoveException, ActionneurException, ScriptException
	{
		try {
			if (!robot.isCargoEmpty()) {
				robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, 0., 228., 20., 1023., 300., 300.);
				robot.HACK_setRobotNonDeploye();
			} else {
				robot.rangeSiPossible();
			}
			robot.goTo(new XYO(positionGoTo, angleGoTo));
			robot.avanceTo(new XYO(positionAvance, angleAvance));
			table.setDone(at);
			if (!robot.isCargoEmpty()) {	
				robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, 0., 228., -60., 1023., 300., 1023.);
				robot.emptyCargoOnBalance();
				robot.execute(CommProtocol.Id.ACTUATOR_GO_TO, 0., 228., 0.);
			}
			robot.avance(-250);
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO, 0., 10., -2.);
			robot.avance(250);
			robot.updateScore(TypeAtome.Greenium.nbPoints);
			robot.avance(-250);
			robot.rangeSiPossible();
			// si tout s'est bien passé, alors le script n'est plus faisable
			done = true;
		}
		catch (PathfindingException e) {
			log.write("Pathfinding exception lors du goTo d'entrée", Subject.SCRIPT);
			throw new ScriptException();
		}
		finally
		{
			try {
				robot.goTo(new XYO(positionEntree, angleEntree));
			} catch (PathfindingException e) {
				log.write("Pathfinding exception lors du goTo de sortie", Subject.SCRIPT);
				throw new ScriptException();
			}
		}
	}
	
	@Override
	public boolean faisable()
	{
		return !done;
	}
	
	public boolean reverseSearch()
	{
		return true;
	}

}
