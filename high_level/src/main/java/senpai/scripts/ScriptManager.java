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

import java.util.Comparator;
import java.util.PriorityQueue;

import pfg.log.Log;
import senpai.capteurs.CapteursProcess;
import senpai.obstacles.ObstaclesDynamiques;
import senpai.robot.Robot;
import senpai.robot.RobotColor;
import senpai.table.AtomeParTerre;
import senpai.table.Table;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Subject;
import pfg.config.Config;
import pfg.kraken.utils.XYO;
import pfg.kraken.utils.XY_RW;

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
	private ObstaclesDynamiques obsDyn;
	
	public void setCouleur(RobotColor couleur)
	{
		this.couleur = couleur;
	}
	
	public ScriptManager(Log log, Config config, Table table, Robot robot, CapteursProcess cp, ObstaclesDynamiques obsDyn)
	{
		this.obsDyn = obsDyn;
		this.log = log;
		this.table = table;
		this.robot = robot;
		this.cp = cp;
	}
	
	public ScriptRecalage getScriptRecalage()
	{
		return new ScriptRecalage(log, robot, table, cp, couleur.symmetry, 500);		
	}
	
	public ScriptAccelerateur getScriptAccelerateur()
	{
		return new ScriptAccelerateur(log, robot, table, cp);
	}
	
	public ScriptRecupereGold getScriptRecupereGold()
	{
		return new ScriptRecupereGold(log, robot, table, cp);
	}
	
	public ScriptRecupereDistrib getScriptRecupereDistrib()
	{
		return new ScriptRecupereDistrib(log, robot, table, cp);
	}
	
	public ScriptDeposeBalance getScriptDeposeBalance()
	{
		return new ScriptDeposeBalance(log, robot, table, cp);
	}
}

