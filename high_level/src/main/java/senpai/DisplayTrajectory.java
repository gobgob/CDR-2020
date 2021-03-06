package senpai;
import java.awt.Color;
import java.util.List;
import pfg.graphic.DebugTool;
import pfg.graphic.printable.Layer;
import pfg.kraken.display.Display;
import pfg.kraken.struct.ItineraryPoint;
import pfg.kraken.struct.XY;
import pfg.log.Log;
import senpai.pathlibrary.KnownPathManager;
import senpai.utils.Severity;
import senpai.utils.Subject;

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
			System.out.println("Usage : ./run.sh "+DisplayTrajectory.class.getSimpleName()+" chemin");
			return;
		}
		
		String configfile = "senpai-trajectory.conf";
		Log log = new Log(Severity.INFO, configfile, "log");
		
		DebugTool debug = DebugTool.getDebugTool(new XY(0,1000), new XY(0, 1000), null, "kraken-examples.conf", "trajectory");
		Display display = debug.getDisplay();

		Color[] couleurs = new Color[]{Color.BLACK, Color.RED, Color.BLUE, Color.GRAY, Color.ORANGE, Color.MAGENTA};
		
		for(int i = 0; i < args.length; i++)
		{
			String filename = args[i];
			KnownPathManager manager = new KnownPathManager(log, null);
			manager.loadAllPaths();
			String f = filename.substring(filename.lastIndexOf("/")+1, filename.length());
			List<ItineraryPoint> path = manager.loadPath(f).path;
				
			for(ItineraryPoint p : path)
			{
				log.write(p, Subject.STATUS);
				System.out.println(p);
				display.addPrintable(p, couleurs[i % couleurs.length], Layer.FOREGROUND.layer);
			}
		}
		display.refresh();

	}
	
}
