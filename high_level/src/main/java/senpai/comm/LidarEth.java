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
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import pfg.config.Config;
import pfg.kraken.obstacles.CircularObstacle;
import pfg.log.Log;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Severity;
import senpai.utils.Subject;

/**
 * Une connexion Ethernet pour le lidar
 * TODO
 * @author pf
 *
 */

public class LidarEth
{
	private Log log;
	private int port;
	private InetAddress adresse;
	private Socket socket;

	private OutputStream output;
	private InputStream input;
	
	public LidarEth(Log log)
	{
		this.log = log;
	}
	
	/**
	 * Constructeur pour la série de test
	 * 
	 * @param log
	 */
	public void initialize(Config config)
	{

	}
	
	private boolean isClosed()
	{
		return socket == null || !socket.isConnected() || socket.isClosed();
	}
	
	
	/**
	 * Doit être appelé quand on arrête de se servir de la communication
	 */
	public synchronized void close()
	{
		if(socket == null)
			return;
		
		assert socket.isConnected() && !socket.isClosed() : "État du socket : "+socket.isConnected()+" "+socket.isClosed();
		
		if(socket.isConnected() && !socket.isClosed())
		{
			try
			{
				log.write("Fermeture de la communication", Subject.COMM);
				socket.close();
				output.close();
			}
			catch(IOException e)
			{
				log.write(e, Severity.WARNING, Subject.COMM);
			}
		}
		else if(socket.isClosed())
			log.write("Fermeture impossible : carte déjà fermée", Severity.WARNING, Subject.COMM);
		else// if(!socket.isConnected())
			log.write("Fermeture impossible : carte jamais ouverte", Severity.WARNING, Subject.COMM);
	}

	public CircularObstacle getObstacle()
	{
		return null;
	}
}
