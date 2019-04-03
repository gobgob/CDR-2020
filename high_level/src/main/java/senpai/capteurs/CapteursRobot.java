package senpai.capteurs;

import pfg.kraken.obstacles.RectangularObstacle;
import pfg.kraken.utils.XY;

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


/**
 * Énum contenant les capteurs du robot
 * 
 * @author pf
 *
 */

public enum CapteursRobot
{
	// TODO: l'ordre doit être cohérent avec la doc
	// TODO: largeur
	
	ToF_AVANT_GAUCHE(new XY(58, 99), 0, TypeCapteur.ToF_LONG, 166),

	ToF_AVANT_DROIT(new XY(58, -99), 0, TypeCapteur.ToF_LONG, 166),

	ToF_COIN_ARRIERE_GAUCHE(new XY(-204, 97), 145. * Math.PI / 180., TypeCapteur.ToF_LONG, 206),

	ToF_COIN_ARRIERE_DROIT(new XY(-204, -97), -145. * Math.PI / 180., TypeCapteur.ToF_LONG, 206),

	ToF_ARRIERE_GAUCHE(new XY(-229, 71), -Math.PI, TypeCapteur.ToF_LONG, 206),

	ToF_ARRIERE_DROITE(new XY(-229, -71), -Math.PI, TypeCapteur.ToF_LONG, 206),

	ToF_FOURCHE_GAUCHE(new XY(106+157, 48), 0, TypeCapteur.ToF_COURT, 206),
	
	ToF_FOURCHE_DROITE(new XY(106+157, -48), 0, TypeCapteur.ToF_COURT, 206);
	
	public final XY pos;
	public final double angle;
	public final TypeCapteur type;
	public final boolean sureleve;
	public RectangularObstacle current;
	public volatile boolean isThereObstacle = false;
	public static final int profondeur = 200;
	public final static CapteursRobot[] values = values();

	private <S extends Capteur> CapteursRobot(XY pos, double angle, TypeCapteur type, int largeur)
	{
		current = new RectangularObstacle(new XY(0,0), profondeur, largeur, 0);
		sureleve = false;
		this.pos = pos;
		this.angle = angle;
		this.type = type;
	}

	public void updateObstacle(XY positionEnnemi, double orientation)
	{
		current.update(positionEnnemi, orientation);
	}
	
	
}
