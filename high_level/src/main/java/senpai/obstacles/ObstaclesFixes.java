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

import pfg.kraken.obstacles.Obstacle;
import pfg.kraken.obstacles.RectangularObstacle;
import pfg.kraken.obstacles.CircularObstacle;
import pfg.kraken.utils.XY;

/**
 * Enumération des obstacles fixes.
 * Afin que les obstacles fixes soient facilement modifiables d'une coupe à
 * l'autre.
 * 
 * @author pf
 *
 */

public enum ObstaclesFixes
{
	// bords
	BORD_BAS(new RectangularObstacle(new XY(0, 0), 3000, 5), true),
	BORD_GAUCHE(new RectangularObstacle(new XY(-1500, 1000), 5, 2000), true),
	BORD_DROITE(new RectangularObstacle(new XY(1500, 1000), 5, 2000), true),
	BORD_HAUT(new RectangularObstacle(new XY(0, 2000), 3000, 5), true),

	ZONE_CHAOS_GAUCHE(new CircularObstacle(new XY(-500, 950), 150), false),
	ZONE_CHAOS_DROITE(new CircularObstacle(new XY(500, 950), 150), false),

	DISTRIBUTEUR_GAUCHE(new RectangularObstacle(new XY(-661, 428), 780, 60), true),
	DISTRIBUTEUR_DROITE(new RectangularObstacle(new XY(661, 428), 780, 60), true),
	
	BALANCES(new RectangularObstacle(new XY(0, 200), 544, 400), true),
	TASSEAU_CENTRAL(new RectangularObstacle(new XY(0, 500), 40, 200), true);
	
	public final Obstacle obstacle;
	public final boolean visible;

	private ObstaclesFixes(Obstacle obstacle, boolean visible)
	{
		this.obstacle = obstacle;
		this.visible = visible;
	}

}