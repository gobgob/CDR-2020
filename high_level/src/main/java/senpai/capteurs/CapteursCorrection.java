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

package senpai.capteurs;

import java.util.ArrayList;
import java.util.List;

/**
 * Les différents capteurs utilisés pour la correction
 * @author pf
 *
 */

public enum CapteursCorrection {

	AVANT(CapteursRobot.ToF_AVANT_GAUCHE,
			CapteursRobot.ToF_AVANT_DROIT, CapteursRobot.ToF_AVANT_DROIT.pos.getX());
	
	public final CapteursRobot c1, c2;
	public volatile boolean enable = false;
	public final double distanceToRobot;
	public List<Integer> valc1 = new ArrayList<Integer>();	
	public List<Integer> valc2 = new ArrayList<Integer>();
	
	private CapteursCorrection(CapteursRobot c1, CapteursRobot c2, double distanceToRobot)
	{
		this.distanceToRobot = distanceToRobot;
		this.c1 = c1;
		this.c2 = c2;
	}
	
}
