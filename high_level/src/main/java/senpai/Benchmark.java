package senpai;

import java.util.ArrayList;
import java.util.List;
import pfg.config.Config;
import pfg.kraken.Kraken;
import pfg.kraken.KrakenParameters;
import pfg.kraken.SearchParameters;
import pfg.kraken.exceptions.PathfindingException;
import pfg.kraken.obstacles.Obstacle;
import pfg.kraken.obstacles.RectangularObstacle;
import pfg.kraken.struct.XY;
import pfg.kraken.struct.XYO;
import pfg.log.Log;
import senpai.scripts.GameState.ObstaclesFixes;
import senpai.threads.ThreadWarmUp;
import senpai.utils.ConfigInfoSenpai;
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
 * Benchmark
 * @author pf
 *
 */

public class Benchmark
{
	public static void main(String[] args) throws NumberFormatException, InterruptedException
	{
		String configfile = "warmup.conf";
		Log log = new Log(Severity.INFO, configfile, "default");
		Config config = new Config(ConfigInfoSenpai.values(), false, configfile, "default");
		
		boolean modeXY = args[0].equals("XY");
		
		if(args.length > 1)
		{
			int duree = Integer.parseInt(args[1]);
			config.override(ConfigInfoSenpai.WARM_UP_DURATION, duree);
		}
		
		log.write("Type de benchmark : "+(modeXY ? "XY" : "XYO"), Subject.STATUS);
		
		log.write("Durée du warm-up : "+config.getInt(ConfigInfoSenpai.WARM_UP_DURATION), Subject.STATUS);
		int demieLargeurNonDeploye = config.getInt(ConfigInfoSenpai.LARGEUR_NON_DEPLOYE) / 2;
		int demieLongueurArriere = config.getInt(ConfigInfoSenpai.DEMI_LONGUEUR_NON_DEPLOYE_ARRIERE);
		int demieLongueurAvant = config.getInt(ConfigInfoSenpai.DEMI_LONGUEUR_DEPLOYE_AVANT);
		List<Obstacle> obstaclesFixes = new ArrayList<Obstacle>();
		for(ObstaclesFixes o : ObstaclesFixes.values())
			obstaclesFixes.add(o.obstacle);

		RectangularObstacle robotTemplate = new RectangularObstacle(demieLargeurNonDeploye, demieLargeurNonDeploye, demieLongueurArriere, demieLongueurAvant);

		KrakenParameters kp = new KrakenParameters(robotTemplate, new XY(-1500, 0), new XY(1500, 2000), configfile, "default");
		kp.setFixedObstacles(obstaclesFixes);
		Kraken kraken = new Kraken(kp);
		ThreadWarmUp warmUp = new ThreadWarmUp(log, kraken, config);
		warmUp.run();
		System.out.println("Warm-up");
		Thread.sleep(config.getInt(ConfigInfoSenpai.WARM_UP_DURATION));
		System.out.println("Wait 2mn");
		try
		{
			double nbIter = 0;
			long before = System.currentTimeMillis();
			do
			{
				if(modeXY)
					kraken.initializeNewSearch(new SearchParameters(new XYO(-500, 700, 2./3.*Math.PI), new XY(1000, 1300)));
				else
					kraken.initializeNewSearch(new SearchParameters(new XYO(1190, 1400, Math.PI), new XYO(-600, 1600, -Math.PI/2)));
				kraken.search();
				nbIter++;
			} while(System.currentTimeMillis() - before < 120000);
			long after = System.currentTimeMillis();
			log.write("Durée moyenne d'une recherche : "+(after - before) / nbIter, Subject.STATUS);
			System.out.println("Durée moyenne d'une recherche : "+(after - before) / nbIter);
		}
		catch(PathfindingException e)
		{
			log.write("Erreur lors du benchmark : "+e, Severity.CRITICAL, Subject.STATUS);
		}

	}
}
