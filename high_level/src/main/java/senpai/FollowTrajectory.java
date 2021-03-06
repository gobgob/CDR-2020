package senpai;

import pfg.log.Log;
import senpai.Senpai.ErrorCode;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.comm.DataTicket;
import senpai.comm.Ticket;
import senpai.pathlibrary.KnownPathManager;
import senpai.pathlibrary.SavedPath;
import senpai.threads.comm.ThreadCommProcess;
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
 * Permet de lancer facilement un test
 * @author pf
 *
 */

public class FollowTrajectory
{
	public static void main(String[] args)
	{
		if(args.length < 1 || args.length > 2)
		{
			System.out.println("Usage : ./run.sh "+FollowTrajectory.class.getSimpleName()+" chemin [maxSpeed]");
			return;
		}
		
		String configfile = "senpai-trajectory.conf";
		
		String filename = args[0];			
		filename = filename.substring(filename.lastIndexOf("/")+1, filename.length());

		Senpai senpai = new Senpai();
		ErrorCode error = ErrorCode.NO_ERROR;
		try
		{
			Log log = new Log(Severity.INFO, configfile, "");
			
			KnownPathManager manager = new KnownPathManager(log, null);
			manager.loadAllPaths();
			SavedPath s = manager.loadPath(filename);
			if(args.length > 1)
				s = manager.limitMaxSpeed(s, Double.parseDouble(args[1]) / 1000.);

			senpai = new Senpai();
			senpai.initialize(configfile, "default");
			senpai.getService(ThreadCommProcess.class).capteursOn = true;
			OutgoingOrderBuffer data = senpai.getService(OutgoingOrderBuffer.class);

			data.setPosition(s.sp.start.getPosition(), s.sp.start.orientationReelle);
			Thread.sleep(1000);
			
			data.ajoutePointsTrajectoire(s.path, true);
			Ticket t = data.followTrajectory();
			DataTicket state = t.attendStatus();
			log.write("Code de retour reçu : "+state, Subject.TRAJECTORY);
		}
		catch(Exception e)
		{
			e.printStackTrace();
			error = ErrorCode.EXCEPTION;
			error.setException(e);
		}
		finally
		{
			try
			{
				senpai.destructor(error);
			}
			catch(InterruptedException e)
			{
				e.printStackTrace();
			}
		}
	}
}
