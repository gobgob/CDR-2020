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

import pfg.kraken.utils.XYO;
import pfg.kraken.utils.XY_RW;
import pfg.log.Log;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.capteurs.CapteursCorrection;
import senpai.capteurs.CapteursProcess;
import senpai.comm.CommProtocol;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.ScriptException;
import senpai.exceptions.UnableToMoveException;
import senpai.robot.Robot;
import senpai.table.Table;

/**
 * Script de l'accelerateur
 * @author pf
 *
 */

public class ScriptAccelerateur extends Script
{
	private XY_RW positionEntree = new XY_RW(-125,1690);
	private double act_y = 23.795;
	private boolean done = false;
	
	public ScriptAccelerateur(Log log, Robot robot, Table table, CapteursProcess cp, OutgoingOrderBuffer out, boolean symetrie)
	{
		super(log, robot, table, cp, out);
		if(symetrie) {
			positionEntree.setX(- positionEntree.getX());
			act_y = -act_y;
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
		return new XYO(positionEntree, Math.PI / 2);
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
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO, act_y, 157., 2.);
			robot.avanceTo(new XYO(positionEntree.getX(), 1740, Math.PI / 2));
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, -act_y, 180., 0., 350., 300., 1023.);
			robot.updateScore(20);
			robot.setGoldeniumFree();
			// si tout s'est bien passé, alors le script n'est plus faisable
			done = true;
		}
		finally
		{
			// dans tous les cas, on recule et on replie l'actionneur
			robot.avance(-100);
		}
	}
	
	@Override
	public boolean faisable()
	{
		return !done;
	}
	
	@Override
	public double getToleranceAngle()
	{
		return 4;
	}
	
	@Override
	public double getTolerancePosition()
	{
		return 15;
	}
	
}
