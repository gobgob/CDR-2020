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
import java.util.Iterator;
import java.util.List;
import pfg.config.Config;
import pfg.graphic.printable.Layer;
import pfg.kraken.display.Display;
import pfg.kraken.display.Printable;
import pfg.kraken.obstacles.CircularObstacle;
import pfg.kraken.obstacles.Obstacle;
import pfg.kraken.obstacles.RectangularObstacle;
import pfg.kraken.struct.XY;
import pfg.log.Log;
import senpai.utils.ConfigInfoSenpai;

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
		BORD_HAUT(new RectangularObstacle(new XY(0, 2000), 3000, 5), true),
		ROCHER_GAUCHE(new RectangularObstacle(new XY(-611, 75), 22, 150), false), // peut-être mal calé en x ?
		ROCHER_MILIEU(new RectangularObstacle(new XY(0, 150), 22, 300), false),
		ROCHER_DROIT(new RectangularObstacle(new XY(611, 75), 22, 150), false);

		public final Obstacle obstacle;
		public final boolean visible;

		private ObstaclesFixes(Obstacle obstacle, boolean visible)
		{
			this.obstacle = obstacle;
			this.visible = visible;
		}

	}

	public enum CouleurBouee {
		ROUGE,
		VERT;
	}
	
	public enum Bouees {
		A(-1200,1600,CouleurBouee.ROUGE),
		B(-1200, 800,CouleurBouee.VERT),
		C(-1050, 1490,CouleurBouee.VERT),
		D(-1050, 920,CouleurBouee.ROUGE),
		E(-830,1900,CouleurBouee.ROUGE),
		F(-550, 1600,CouleurBouee.VERT),
		G(-400, 1200,CouleurBouee.ROUGE),
		H(-230,800,CouleurBouee.VERT),
		I(-495,45,CouleurBouee.ROUGE),
		J(-435,350, CouleurBouee.VERT),
		K(-165,350, CouleurBouee.ROUGE),
		L(-105,45, CouleurBouee.VERT),
		A2(1200,1600,CouleurBouee.VERT),
		B2(1200, 800,CouleurBouee.ROUGE),
		C2(1050, 1490, CouleurBouee.ROUGE),
		D2(1050, 920,CouleurBouee.VERT),
		E2(830,1900, CouleurBouee.VERT),
		F2(550, 1600, CouleurBouee.ROUGE),
		G2(400, 1200, CouleurBouee.VERT),
		H2(230,800, CouleurBouee.ROUGE),
		I2(495,45,CouleurBouee.VERT),
		J2(435,350,CouleurBouee.ROUGE),
		K2(165,350,CouleurBouee.VERT),
		L2(105,45,CouleurBouee.ROUGE);
		
		public final Obstacle obs;
		public static final int diametreBouee = 72;
		public boolean surTable = true;
		public final CouleurBouee couleur;
		
		private Bouees(int x, int y, CouleurBouee c)
		{
			obs = new CircularObstacle(new XY(x, y), diametreBouee/2);
			couleur = c;
		}
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
		for(Bouees n : Bouees.values())
			if(n.surTable)
				currentObstacles.add(n.obs);
		currentObstacles.addAll(otherObstacles);
		return currentObstacles.iterator();
	}
	
	@Override
	public void print(Graphics g, Display f)
	{
		g.setColor(Color.BLUE);
		for(ObstaclesFixes o : ObstaclesFixes.values())
			o.obstacle.print(g, f);
		for(Bouees n : Bouees.values())
		{
			if(n.couleur == CouleurBouee.ROUGE)
				g.setColor(Color.RED.darker().darker());
			else
				g.setColor(Color.GREEN.darker().darker());
			n.obs.print(g, f);
		}
	}
}