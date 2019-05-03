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
import senpai.capteurs.CapteursProcess;
import senpai.comm.CommProtocol;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.ScriptException;
import senpai.exceptions.UnableToMoveException;
import senpai.robot.Robot;
import senpai.table.Table;
import senpai.table.TypeAtome;

/**
 * Script de récupération du goldenium
 * @author pf
 *
 */

public class ScriptRecupereGold extends Script
{
	private XY_RW positionEntree = new XY_RW(-725,1665);
	private boolean done = false;
	
	public ScriptRecupereGold(Log log, Robot robot, Table table, CapteursProcess cp)
	{
		super(log, robot, table, cp);
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
		boolean success = false;
		try {
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO, -23.7, 200., 0.);
			Double y = (Double) robot.execute(CommProtocol.Id.ACTUATOR_FIND_PUCK);
			if(y == null)
				throw new ActionneurException("No y after actuator find puck ?!", 0);
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO, y, 180., 0.);
			robot.avance(75);
			done = true;
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, y, 180., 15., 1023., 300., 900.);
			success = true;
			// le script n'est plus faisable
		}
		finally
		{
			robot.avance(-75);
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, 0, 180., 15., 1023., 300., 900.);
			if(success)
				robot.addToCargo(TypeAtome.Goldenium);
		}
	}
	
	@Override
	public boolean faisable()
	{
		return !done && !robot.isCargoFull();
	}
}
