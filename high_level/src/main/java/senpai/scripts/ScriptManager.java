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

import pfg.log.Log;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.capteurs.CapteursProcess;
import senpai.robot.Robot;
import senpai.robot.RobotColor;
import senpai.table.Table;

/**
 * Script manager
 * @author pf
 *
 */

public class ScriptManager
{
	protected Log log;
	private Table table;
	private Robot robot;
	private CapteursProcess cp;
	private RobotColor couleur;
	private OutgoingOrderBuffer out;
	
	public void setCouleur(RobotColor couleur)
	{
		this.couleur = couleur;
	}
	
	public ScriptManager(Log log, Table table, Robot robot, CapteursProcess cp, OutgoingOrderBuffer out)
	{
		this.log = log;
		this.table = table;
		this.robot = robot;
		this.cp = cp;
		this.out = out;
	}
	
	public ScriptAccelerateur getScriptAccelerateur()
	{
		return new ScriptAccelerateur(log, robot, table, cp, out, couleur.symmetry);
	}
	
	public ScriptRecupereGold getScriptRecupereGold()
	{
		return new ScriptRecupereGold(log, robot, table, cp, out, couleur.symmetry);
	}
	
	public ScriptRecuperePalet getScriptRecuperePalet()
	{
		return new ScriptRecuperePalet(log, robot, table, cp, out, couleur.symmetry);
	}
	
	public ScriptMonteRampe getScriptMonteRampe()
	{
		return new ScriptMonteRampe(log, robot, table, cp, out, couleur.symmetry);
	}
	
	public ScriptPousseAtomeHaut getScriptPousseAtomeHaut()
	{
		return new ScriptPousseAtomeHaut(log, robot, table, cp, out, couleur.symmetry);
	}

	public ScriptPousseAtomeBas getScriptPousseAtomeBas()
	{
		return new ScriptPousseAtomeBas(log, robot, table, cp, out, couleur.symmetry);
	}

	public ScriptPousseAtomeMilieu getScriptPousseAtomeMilieu()
	{
		return new ScriptPousseAtomeMilieu(log, robot, table, cp, out, couleur.symmetry);
	}

	public ScriptDeposeBalance getScriptDeposeBalance()
	{
		return new ScriptDeposeBalance(log, robot, table, cp, out, couleur.symmetry);
	}
}

