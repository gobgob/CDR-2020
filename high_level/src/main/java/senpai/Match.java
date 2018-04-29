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

package senpai;

import pfg.config.Config;
import pfg.kraken.Kraken;
import pfg.log.Log;
import senpai.Senpai.ErrorCode;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.comm.CommProtocol;
import senpai.comm.DataTicket;
import senpai.comm.Ticket;
import senpai.robot.Robot;
import senpai.utils.Subject;

/**
 * Code d'un match
 * @author pf
 *
 */

public class Match {

	public static void main(String[] args) throws InterruptedException
	{
		Senpai senpai = new Senpai();
		senpai.initialize("match.conf", args);
		ErrorCode error = ErrorCode.NO_ERROR;
		try {
			Log log = senpai.getService(Log.class);
			Config config = senpai.getService(Config.class);
			OutgoingOrderBuffer data = senpai.getService(OutgoingOrderBuffer.class);
			Robot robot = senpai.getService(Robot.class);
			Kraken kraken = senpai.getService(Kraken.class);
			
			Ticket t = data.waitForJumper();
			
			log.write("Attente de la couleur…", Subject.STATUS);
	
			
			DataTicket etat;
			do
			{
				// Demande la couleur toute les 100ms et s'arrête dès qu'elle est connue
				Ticket tc = data.demandeCouleur();
				etat = tc.attendStatus();
				Thread.sleep(100);
			} while(etat.status != CommProtocol.State.OK);
			
			
			/*
			 * Stratégie:
			 * golden cube dans le coffre
			 * on va prendre le cube du bas
			 * on pose le cube qu'on tenait puis le golden cube
			 */
			
			System.out.println("Code du match !");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			error = ErrorCode.EXCEPTION;
		}
		finally
		{
			senpai.destructor(error);
		}
	}
	
}
