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

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.reflect.Method;
import org.junit.Before;
import org.junit.Test;

import senpai.buffer.OutgoingOrderBuffer;
import senpai.capteurs.CapteursProcess;
import senpai.comm.CommProtocol.Id;
import senpai.robot.Robot;
import senpai.scripts.GameState;
import senpai.scripts.ScriptManager;

/**
 * Tests unitaires des trajectoires et des actionneurs
 * 
 * @author pf
 *
 */

public class Test_Robot extends JUnit_Test
{

//	private AStarCourbe astar;
//	private CheminPathfinding chemin;
//	private RealGameState state;
//	private PathCache pathcache;
	private OutgoingOrderBuffer data;
//	private Cinematique c = null;
//	private boolean simuleSerie;
//	private double last;
	

	/**
	 * Génère un fichier qui présente les tests
	 * 
	 * @param args
	 */
	public static void main(String[] args)
	{
		BufferedWriter writer = null;
		try
		{
			writer = new BufferedWriter(new FileWriter("liste-tests.txt"));
			Method[] methodes = Test_Robot.class.getDeclaredMethods();
			for(Method m : methodes)
				if(m.isAnnotationPresent(Test.class))
					writer.write("./run_junit.sh tests.lowlevel.JUnit_Robot#" + m.getName() + "\n");
		}
		catch(IOException e)
		{
			e.printStackTrace();
		}
		finally
		{
			if(writer != null)
				try
				{
					writer.close();
				}
				catch(IOException e)
				{
					e.printStackTrace();
				}
			System.out.println("Génération de la liste des tests terminée.");
		}
	}

	/**
	 * Pas un test
	 */
	@Before
	public void setUp() throws Exception
	{
		setUp("default");
		
		// il est nécessaire que les communications ne soient pas simulées
//		assert !config.getBoolean(ConfigInfoSenpai.SIMULE_COMM);

//		state = container.getService(RealGameState.class);
		container.getService(Robot.class);
//		chemin = container.getService(CheminPathfinding.class);
//		astar = container.getService(AStarCourbe.class);
//		pathcache = container.getService(PathCache.class);
		data = container.getService(OutgoingOrderBuffer.class);
		container.getService(ScriptManager.class);
		container.getService(CapteursProcess.class);
		container.getService(GameState.class);
//		simuleSerie = config.getBoolean(ConfigInfoSenpai.SIMULE_COMM);
		data.startStream(Id.ODO_AND_SENSORS);
	}

}
