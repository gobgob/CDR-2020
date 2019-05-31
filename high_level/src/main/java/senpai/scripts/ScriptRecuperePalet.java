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
import senpai.capteurs.CapteursProcess;
import senpai.capteurs.CapteursCorrection;
import senpai.comm.CommProtocol;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.ScriptException;
import senpai.exceptions.UnableToMoveException;
import senpai.robot.Robot;
import senpai.table.Table;
import senpai.table.TypeAtome;

/**
 * Script de récupération d'un palet
 * @author pf
 *
 */

public class ScriptRecuperePalet extends Script
{
	private XY_RW positionEntree;
	private TypeAtome atome;
	private boolean done = false;
	
	public ScriptRecuperePalet(Log log, Robot robot, Table table, CapteursProcess cp, OutgoingOrderBuffer out,
		boolean symetrie, XY_RW entryPoint, TypeAtome tAtome)
	{
		super(log, robot, table, cp, out);
		positionEntree = entryPoint;
		atome = tAtome;
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
		return new XYO(positionEntree, -Math.PI / 2);
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
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, -23.7, 107., 2., 1023., 300., 300.);
			Object[] d = (Object[]) robot.execute(CommProtocol.Id.ACTUATOR_FIND_PUCK, Boolean.FALSE);
			if(d == null)
				throw new ActionneurException("No data after actuator find puck ?!", 0);
			double y = (double) d[0];
			int code = (int) d[2];
			if(code != 0)
				throw new ActionneurException("No detection!", code);
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, y, 107., 2., 1023., 300., 300.);
			robot.avanceTo(new XYO(positionEntree.getX(), positionEntree.getY() - 55, -Math.PI / 2));
			robot.execute(CommProtocol.Id.ACTUATOR_GO_TO_AT_SPEED, y, 120., 40., 1023., 300., 100.);
			robot.addToCargo(atome);
			done = true; // le script n'est plus faisable
		}
		finally
		{
			robot.avance(-100);
		}
	}
	
	@Override
	public boolean faisable()
	{
		return !done && !robot.isCargoFull(atome) && robot.isScriptPousseAtomeHautFait();
	}
}
