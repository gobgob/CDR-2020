package senpai;

import pfg.kraken.struct.XY;
import senpai.Senpai.ErrorCode;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.robot.Robot;
import senpai.robot.RobotColor;
import senpai.threads.comm.ThreadCommProcess;

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

public class TestAvance
{
	public static void main(String[] args)
	{
		String configfile = "senpai-trajectory.conf";
		

		Senpai senpai = new Senpai();
		ErrorCode error = ErrorCode.NO_ERROR;
		try
		{
			int distance = Integer.parseInt(args[0]);
			double vitesse = Double.parseDouble(args[1]);
			senpai = new Senpai();
			senpai.initialize(configfile, "default", "graphic");
			OutgoingOrderBuffer data = senpai.getService(OutgoingOrderBuffer.class);
			data.setPosition(new XY(0, 1000), 0);
			Thread.sleep(1000);
			senpai.getService(ThreadCommProcess.class).capteursOn = true;
			Robot robot = senpai.getService(Robot.class);
			robot.updateColorAndSendPosition(RobotColor.VIOLET);
			robot.avance(distance, vitesse);
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
