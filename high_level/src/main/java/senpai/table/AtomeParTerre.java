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

package senpai.table;

import pfg.kraken.obstacles.CircularObstacle;
import pfg.kraken.obstacles.Obstacle;
import pfg.kraken.utils.XY;

/**
 * Enumérations contenant tous les éléments de jeux
 * 
 * @author pf
 *
 */

public enum AtomeParTerre
{
	MILIEU_GAUCHE(new CircularObstacle(new XY(-1000, 1250-10), 76/2-10)),
	HAUT_GAUCHE(new CircularObstacle(new XY(-1000, 1550+10), 76/2-10)),

	MILIEU_DROITE(new CircularObstacle(new XY(1000, 1250-10), 76/2-10)),
	HAUT_DROITE(new CircularObstacle(new XY(1000, 1550+10), 76/2-10)),

	PENTE_GAUCHE(new CircularObstacle(new XY(-666, 200), 38)),
	PENTE_DROITE(new CircularObstacle(new XY(666, 200), 38));

	public final Obstacle obstacle;// pour vérifier les collisions

	private AtomeParTerre(Obstacle obstacle)
	{
		this.obstacle = obstacle;
	}

}
