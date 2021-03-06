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

import pfg.kraken.struct.XYO;
import pfg.log.Log;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.capteurs.CapteursProcess;
import senpai.exceptions.ActionneurException;
import senpai.exceptions.ScriptException;
import senpai.exceptions.UnableToMoveException;
import senpai.robot.Robot;
import senpai.utils.Severity;
import senpai.utils.Subject;

/**
 * Script abstrait
 * 
 * @author pf
 *
 */

public abstract class Script
{
	protected Log log;
	protected Robot robot;
	protected GameState table;
	protected CapteursProcess cp;
	protected OutgoingOrderBuffer out;
	
	public boolean reverseSearch()
	{
		return false;
	}
	
	public Script(Log log, Robot robot, GameState table, CapteursProcess cp, OutgoingOrderBuffer out)
	{
		this.log = log;
		this.robot = robot;
		this.table = table;
		this.cp = cp;
		this.out = out;
	}
	
	public double getToleranceAngle()
	{
		return 5;
	}
	
	public double getTolerancePosition()
	{
		return 20;
	}
	
	public double getToleranceX()
	{
		return 20;
	}
	
	public double getToleranceY()
	{
		return 20;
	}
	
	private XYO corrected = new XYO(0,0,0);
	
	public XYO correctOdo() throws InterruptedException
	{
		robot.getCinematique().getXYO().copy(corrected);
		return corrected;
	}
	
	public abstract boolean faisable();
	
	public abstract XYO getPointEntree();

	protected abstract void run() throws InterruptedException, UnableToMoveException, ActionneurException, ScriptException;

	public void execute() throws ScriptException, InterruptedException
	{
		log.write("Début de l'exécution de "+this, Subject.SCRIPT);
		try
		{
			run();
			log.write("Fin de l'exécution de " + getClass().getSimpleName(), Subject.SCRIPT);
		}
		catch(ScriptException | UnableToMoveException | ActionneurException e)
		{
			log.write("Erreur lors de l'exécution du script " + this + " : " + e, Severity.CRITICAL, Subject.SCRIPT);
/*			try {
				// range actionneurs
			} catch (ActionneurException e1) {
				log.write("Erreur lors de l'exécution du script " + this + " : " + e, Severity.CRITICAL, Subject.SCRIPT);
			}*/
			throw new ScriptException(e.getMessage());
		}
	}

	@Override
	public int hashCode()
	{
		return toString().hashCode();
	}

	@Override
	public boolean equals(Object o)
	{
		return o != null && o.toString().equals(toString());
	}

}
