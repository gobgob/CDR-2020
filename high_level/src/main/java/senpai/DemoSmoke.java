package senpai;

import senpai.Senpai.ErrorCode;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.comm.CommProtocol;
import senpai.comm.DataTicket;
import senpai.comm.Ticket;

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
 * Fumée
 * @author pf
 *
 */

public class DemoSmoke
{
	public static void main(String[] args)
	{
		String configfile = "demo-smoke.conf";
		
		Senpai senpai = new Senpai();
		ErrorCode error = ErrorCode.NO_ERROR;
		OutgoingOrderBuffer ll = null;
		boolean smokeOn = false;
		try {
			senpai.initialize(configfile, "default");
			ll = senpai.getService(OutgoingOrderBuffer.class);
			ll.setScore(4242);
			while(true)
			{
				Ticket tc = ll.demandeCouleur();
				DataTicket etat = tc.attendStatus();
				Thread.sleep(20);
				if(etat.status != CommProtocol.State.OK && smokeOn)
				{
					smokeOn = false;
					ll.enableSmoke(false);
				}
				else if(etat.status == CommProtocol.State.OK && !smokeOn)
				{
					smokeOn = true;
					ll.enableSmoke(true);
				}
			}
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
				if(ll != null)
				{
					ll.enableSmoke(false);
					ll.setScore(0);
				}
				senpai.destructor(error);
			}
			catch(InterruptedException e)
			{
				e.printStackTrace();
			}
		}
	}
}
