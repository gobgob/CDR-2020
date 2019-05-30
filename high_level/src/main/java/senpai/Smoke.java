package senpai;

import pfg.config.Config;
import pfg.graphic.GraphicDisplay;
import pfg.injector.Injector;
import pfg.injector.InjectorException;
import pfg.log.Log;
import senpai.buffer.OutgoingOrderBuffer;
import senpai.comm.Communication;
import senpai.threads.comm.ThreadCommEmitter;
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
 * Fumée
 * @author pf
 *
 */

public class Smoke
{
	public static void main(String[] args) throws InjectorException, InterruptedException
	{
		String configfile = "match.conf";
		

		int duree = 3000;
		if(args.length > 0)
			duree = Integer.parseInt(args[0]);

		Injector injector = new Injector();
		Subject.STATUS.setShouldPrint(true);
		Log log = new Log(Severity.INFO, configfile, "default");
		Config config = new Config(ConfigInfoSenpai.values(), false, configfile, "default");
		log.write("Démarrage script Smoke", Subject.STATUS);
		if(config.getBoolean(ConfigInfoSenpai.ENABLE_SMOKE))
		{
			log.write("Fumée activée !", Subject.STATUS);
			injector.addService(GraphicDisplay.class, null);
			injector.addService(log);
			injector.addService(config);
			injector.getService(Communication.class).initialize();
			OutgoingOrderBuffer data = injector.getService(OutgoingOrderBuffer.class);
			injector.getService(ThreadCommEmitter.class).start();
			data.enableSmoke(true);
			Thread.sleep(duree);
			data.enableSmoke(false);
		}
		log.write("Script Smoke fini !", Subject.STATUS);


	}
}
