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

import java.awt.Color;
import java.awt.Graphics;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import pfg.config.Config;
import pfg.graphic.printable.Layer;
import pfg.kraken.display.Display;
import pfg.kraken.display.Printable;
import pfg.kraken.obstacles.Obstacle;
import pfg.kraken.obstacles.RectangularObstacle;
import pfg.kraken.struct.XY;
import pfg.log.Log;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Subject;

/**
 * Gère les éléments de jeux
 * 
 * @author pf
 *
 */

public class GameState implements Printable
{
	public enum ObstaclesFixes
	{
		// bords
		BORD_BAS(new RectangularObstacle(new XY(0, 0), 3000, 5), true),
		BORD_GAUCHE(new RectangularObstacle(new XY(-1500, 1000), 5, 2000), true),
		BORD_DROITE(new RectangularObstacle(new XY(1500, 1000), 5, 2000), true),
		BORD_HAUT(new RectangularObstacle(new XY(0, 2000), 3000, 5), true);

		public final Obstacle obstacle;
		public final boolean visible;

		private ObstaclesFixes(Obstacle obstacle, boolean visible)
		{
			this.obstacle = obstacle;
			this.visible = visible;
		}

	}
	
	public enum Bouees {
		
	}
	
	private static final long serialVersionUID = 1L;

	// Dépendances
	protected transient Log log;

	// Utilisé pour construire à la volée les obstacles courants
	private List<Obstacle> currentObstacles = new ArrayList<Obstacle>();
	
	// Ajoute des obstacles à la table
	private List<Obstacle> otherObstacles = new ArrayList<Obstacle>();
	
	// Zones dans lesquelles on sait qu'il ne peut pas y avoir d'ennemis
	private List<Obstacle> noEnemyZone = new ArrayList<Obstacle>();
	
	public GameState(Log log, Config config, Display buffer)
	{
		this.log = log;
		if(config.getBoolean(ConfigInfoSenpai.GRAPHIC_ENABLE))
			buffer.addPrintable(this, Color.BLACK, Layer.BACKGROUND.layer);
	}
	
	public void addOtherObstacle(Obstacle o)
	{
		otherObstacles.add(o);
	}

	public void addNoEnemyZone(Obstacle o)
	{
		noEnemyZone.add(o);
	}

	public List<Obstacle> getNoEnemyZone()
	{
		return noEnemyZone;
	}
	
	public Iterator<Obstacle> getCurrentObstaclesIterator()
	{
		currentObstacles.clear();
/*		for(AtomeParTerre n : AtomeParTerre.values())
			if(!etat.get(n))
				currentObstacles.add(n.obstacle);*/
		currentObstacles.addAll(otherObstacles);
		return currentObstacles.iterator();
	}
	
	@Override
	public void print(Graphics g, Display f)
	{
	}
}