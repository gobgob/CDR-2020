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

package senpai.obstacles;

import java.awt.Color;
import java.awt.Graphics;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import pfg.config.Config;
import pfg.graphic.printable.Layer;
import pfg.kraken.display.Display;
import pfg.kraken.display.Printable;
import pfg.kraken.obstacles.CircularObstacle;
import pfg.kraken.obstacles.Obstacle;
import pfg.kraken.obstacles.RectangularObstacle;
import pfg.kraken.obstacles.container.SmartDynamicObstacles;
import pfg.log.Log;
import senpai.capteurs.CapteursRobot;
import senpai.scripts.GameState;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Severity;
import senpai.utils.Subject;

/**
 * Regroupe les obstacles de capteurs et de table
 * @author pf
 *
 */

public class ObstaclesDynamiques extends SmartDynamicObstacles implements Iterator<Obstacle>, Printable
{
	private transient Log log;
	private static final long serialVersionUID = 1L;
	private transient GameState table;
	private transient Iterator<Obstacle> iteratorMemory, iteratorTable;
	private transient Iterator<CircularObstacle> iteratorLidar;
//	private transient boolean obsTable;
	private transient CircularObstacle[] lidarObs = new CircularObstacle[100];
	private transient List<CircularObstacle> lidarTmpObs = new ArrayList<CircularObstacle>(100);
	
	public ObstaclesDynamiques(Log log, GameState table, Config config, Display buffer)
	{
		this.log = log;
//		obsTable = !config.getBoolean(ConfigInfoSenpai.NO_OBSTACLES);
		this.table = table;
		if(config.getBoolean(ConfigInfoSenpai.GRAPHIC_SEEN_OBSTACLES))
			buffer.addPrintable(this, Color.BLACK, Layer.FOREGROUND.layer);
	}

	@Override
	public Iterator<Obstacle> getCurrentDynamicObstacles()
	{
		List<Obstacle> capt = new ArrayList<Obstacle>();
		for(CapteursRobot c : CapteursRobot.values)
			if(c.isThereObstacle)
				capt.add(c.current);
		iteratorMemory =  capt.iterator();
		iteratorTable = table.getCurrentObstaclesIterator();
		lidarTmpObs.clear();
		for(CircularObstacle obs : lidarObs)
			if(obs != null)
				lidarTmpObs.add(obs);
		iteratorLidar = lidarTmpObs.iterator();
		return this;
	}

	@Override
	public boolean hasNext()
	{
		return iteratorLidar.hasNext() || iteratorMemory.hasNext() || (/*obsTable &&*/ iteratorTable.hasNext());
	}

	@Override
	public Obstacle next()
	{
		if(iteratorLidar.hasNext())
			return iteratorLidar.next();
		if(iteratorMemory.hasNext())
			return iteratorMemory.next();
		return iteratorTable.next();
	}

	@Override
	protected void addObstacle(Obstacle obs)
	{}

	public synchronized int isThereCollision(List<RectangularObstacle> currentPath)
	{
		int i = 0;
		for(RectangularObstacle ro : currentPath)
		{
			for(Obstacle o : newObs)
			{
//				log.write("Vérif "+o+" avec "+ro, Subject.STATUS);
				if(o.isColliding(ro))
				{
					log.write("Collision détectée avec "+o+" depuis "+ro+" : arrêt nécessaire.", Severity.CRITICAL, Subject.STATUS);
					newObs.clear();
					return i;
				}
			}
			i++;
		}
		newObs.clear();
		return i;
	}

	public void update(CapteursRobot c) {
		super.add(c.current);
	}

	@Override
	public void print(Graphics g, Display f)
	{
		for(CapteursRobot c : CapteursRobot.values)
			if(c.isThereObstacle)
				c.current.print(g, f);		
	}

	public void clearNew()
	{
		newObs.clear();
	}

	public boolean collisionScript(RectangularObstacle o)
	{
		Iterator<Obstacle> iter = getCurrentDynamicObstacles();
		while(iter.hasNext())
			if(iter.next().isColliding(o))
				return true;
		return false;
	}

	public synchronized void setLidarObs(CircularObstacle obs, int id)
	{
		lidarObs[id] = obs;
		newObs.add(obs);
		notifyAll();
	}

	public void clearLidarObs()
	{
		for(int i = 0; i < lidarObs.length; i++)
			lidarObs[i] = null;
	}
	
}
