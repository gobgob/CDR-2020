package senpai;

import java.awt.Color;
import java.util.List;
import pfg.graphic.GraphicDisplay;
import pfg.graphic.printable.Layer;
import pfg.kraken.Kraken;
import pfg.kraken.SearchParameters;
import pfg.kraken.astar.DirectionStrategy;
import pfg.kraken.exceptions.PathfindingException;
import pfg.kraken.robot.ItineraryPoint;
import pfg.kraken.utils.XY;
import pfg.kraken.utils.XYO;
import pfg.log.Log;
import senpai.Senpai.ErrorCode;
import senpai.robot.KnownPathManager;
import senpai.robot.SavedPath;
import senpai.utils.Subject;

public class ConstructTrajectoryAllObstacles
{
	public static void main(String[] args) throws InterruptedException
	{
		if(args.length < 8 || args.length > 10)
		{
			System.out.println("Usage : ./run.sh "+ConstructTrajectory.class.getSimpleName()+" mode vitesse_max deploye x_depart y_depart o_depart x_arrivee y_arrivee [o_arrivee] [chemin]");
			return;
		}
		
		boolean modeXY = args[0].equals("XY");
		
		double vitesseMax = Double.parseDouble(args[1]) / 1000.;
		
		boolean deploye = Boolean.parseBoolean(args[2]);
		double x = Double.parseDouble(args[3]);
		double y = Double.parseDouble(args[4]);
		double o = Double.parseDouble(args[5]);
		XYO depart = new XYO(x,y,o);
		
		x = Double.parseDouble(args[6]);
		y = Double.parseDouble(args[7]);
		
		XY arriveeXY = null;
		XYO arriveeXYO = null;
		if(modeXY)
			arriveeXY = new XY(x,y);
		else
		{
			o = Double.parseDouble(args[8]);
			arriveeXYO = new XYO(x,y,o);
		}
		
		String output = null;
		if(modeXY && args.length > 8)
			output = args[8];
		else if(!modeXY && args.length > 9)
			output = args[9];		
		
		Senpai senpai = new Senpai();
		senpai.initialize("senpai-trajectory.conf", "default", "graphic");
		Kraken k = senpai.getService(Kraken[].class)[deploye ? 0 : 1];
		Log log = senpai.getService(Log.class);
		GraphicDisplay display = senpai.getService(GraphicDisplay.class);
		display.refresh();
		try
		{
			SearchParameters sp;
			if(modeXY)
				sp = new SearchParameters(depart, arriveeXY);
			else
				sp = new SearchParameters(depart, arriveeXYO);
			sp.setDirectionStrategy(DirectionStrategy.FASTEST);
			sp.setMaxSpeed(vitesseMax);
			k.initializeNewSearch(sp);
			
			List<ItineraryPoint> path = k.search();
			for(ItineraryPoint p : path)
			{
				log.write(p, Subject.STATUS);
				display.addPrintable(p, Color.BLACK, Layer.FOREGROUND.layer);
			}
			display.refresh();
			if(output != null)
			{
				KnownPathManager manager = new KnownPathManager(log, null);
				manager.savePath(new SavedPath(path, sp, output));
			}
			else
				log.write("Chemin non sauvegardé", Subject.STATUS);
		}
		catch(PathfindingException e)
		{
			e.printStackTrace();
		}
		senpai.destructor(ErrorCode.NO_ERROR);
	}
}
