package senpai;

import pfg.config.Config;
import senpai.Senpai.ErrorCode;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.utils.ConfigInfoSenpai;

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

public class Smoke
{
	public static void main(String[] args)
	{
		String configfile = "match.conf";
		

		int duree = 3000;
		if(args.length > 0)
			duree = Integer.parseInt(args[0]);

		Senpai senpai = new Senpai();
		ErrorCode error = ErrorCode.NO_ERROR;
		try {
			senpai.initialize(configfile, "default");
			Config config = senpai.getService(Config.class);
			if(config.getBoolean(ConfigInfoSenpai.ENABLE_SMOKE))
			{
				OutgoingOrderBuffer data = senpai.getService(OutgoingOrderBuffer.class);
				data.enableSmoke(true);
				Thread.sleep(duree);
				data.enableSmoke(false);
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
				senpai.destructor(error);
			}
			catch(InterruptedException e)
			{
				e.printStackTrace();
			}
		}
	}
}
