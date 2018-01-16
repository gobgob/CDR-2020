package senpai;
import java.util.List;

import pfg.kraken.robot.ItineraryPoint;

/*
 * Copyright (C) 2013-2017 Pierre-François Gimenez
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
 * Affiche une trajectoire déjà calculée
 * @author pf
 *
 */

public class DisplayTrajectory
{

	public static void main(String[] args)
	{
		if(args.length < 1)
		{
			System.out.println("Usage : ./run.sh "+DisplayTrajectory.class.getSimpleName()+" chemin.path [-v]");
			System.out.println("-v ajoute des infos graphiques");
			return;
		}
		
		boolean verbose = false;
		String filename = args[0];
		if(args.length >= 2 && args[1].equals("-v"))
			verbose = true;
		List<ItineraryPoint> path = KnownPathManager.loadPath(filename);
	}
	
}