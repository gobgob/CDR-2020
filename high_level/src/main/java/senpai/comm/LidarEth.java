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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.InetAddress;
import java.net.UnknownHostException;
import pfg.config.Config;
import pfg.log.Log;
import senpai.robot.RobotColor;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Severity;
import senpai.utils.Subject;

/**
 * Une connexion Ethernet pour le lidar
 * @author pf
 *
 */

public class LidarEth
{
	private Log log;
	private ServerSocket socket = null;
	private Socket client;
	
	private OutputStreamWriter output;
	private BufferedReader input;
	
	public LidarEth(Log log)
	{
		this.log = log;
	}
	
	/**
	 * Constructeur pour la série de test
	 * 
	 * @param log
	 * @throws IOException 
	 */
	public void initialize(Config config) throws IOException
	{
		int port = config.getInt(ConfigInfoSenpai.ETH_LIDAR_PORT_NUMBER);
		try {
			// on accepte une connexion max à la fois			
			socket = new ServerSocket(port, 1, InetAddress.getByName("127.0.0.1"));
		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
		    e.printStackTrace();
		}
		log.write("Attente de la connexion du lidar...", Subject.COMM);
	}
	
	public void waitLidar() throws IOException
	{
		client = socket.accept();
		log.write("Lidar connecté !", Subject.COMM);
		
		input = new BufferedReader(new InputStreamReader(client.getInputStream()));
		output = new OutputStreamWriter(client.getOutputStream());
		assert input != null && output != null;
	}

	/**
	 * Doit être appelé quand on arrête de se servir de la communication
	 */
	public synchronized void close()
	{		
		if(socket == null)
			return;
		
		if(!socket.isClosed())
		{
			try
			{
				log.write("Fermeture de la communication", Subject.COMM);
				socket.close();
			}
			catch(IOException e)
			{
				log.write(e, Severity.WARNING, Subject.COMM);
			}
		}
	}

	public String getMessage() throws InterruptedException, IOException
	{
		return input.readLine();
	}
	
	public void sendInit(RobotColor c) throws IOException
	{
		log.write("Envoi couleur lidar: "+c, Subject.COMM);

		if(c == RobotColor.JAUNE)
			output.write("INIT JAUNE\n");
		else
			output.write("INIT VIOLET\n");
		output.flush();
	}
	
	public void sendOdo() throws IOException
	{
		log.write("Demande correction odo", Subject.COMM);
		output.write("CORRECTION_ODO\n");
		output.flush();
	}
	
	public void sendAck() throws IOException
	{
		output.write("ACK\n");
		output.flush();
	}
	
	public void sendStart() throws IOException
	{
		log.write("Start match lidar", Subject.COMM);
		output.write("START\n");
		output.flush();
	}

	public void sendStop() throws IOException
	{
		log.write("Stop match lidar", Subject.COMM);
		output.write("STOP\n");
		output.flush();
	}
	
}
