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

package senpai.comm;

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import pfg.config.Config;
import pfg.log.Log;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Subject;

/**
 * Une connexion Ethernet l'électron
 * @author pf
 *
 */

public class ElectronEth
{
	private Log log;
	
	public ElectronEth(Log log)
	{
		this.log = log;
	}
	
	/**
	 * Constructeur pour la série de test
	 * 
	 * @param log
	 * @throws IOException 
	 */
	public void previensElectron(Config config) throws IOException
	{
		int port = config.getInt(ConfigInfoSenpai.ETH_ELECTRON_PORT_NUMBER);
		String hostname = config.getString(ConfigInfoSenpai.ETH_HL_HOSTNAME_SERVER);
		ServerSocket socket = null;
		try {
			socket = new ServerSocket(port, 200, InetAddress.getByName(hostname));
		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
		    e.printStackTrace();
		}
		log.write("Prêt à ouvrir le socket de l'électron", Subject.COMM);
		Socket client = socket.accept();
		log.write("Électron connecté", Subject.COMM);
		client.close();
		socket.close();
		log.write("Socket électron fermé.", Subject.COMM);
	}
}
