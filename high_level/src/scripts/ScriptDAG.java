/*
 * Copyright (C) 2013-2017 Pierre-François Gimenez
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

package scripts;

import java.util.List;

import exceptions.ActionneurException;
import exceptions.UnableToMoveException;
import pfg.kraken.robot.Cinematique;
import pfg.kraken.robot.RobotState;

/**
 * Un DAG qui représente un script
 * @author pf
 *
 */

public class ScriptDAG
{
	// TODO assert acyclic
	private List<ScriptDAGNode> graphe;
	private Cinematique pointEntree;
	
	public ScriptDAG(String filename)
	{
		// TODO
	}
	
	public ScriptDAG(List<ScriptDAGNode> graphe)
	{
		assert !graphe.isEmpty();
		this.graphe = graphe;
	}

	protected void run(RobotState state) throws InterruptedException, UnableToMoveException, ActionneurException
	{
		ScriptDAGNode current = graphe.get(0);
		while(current != null)
		{
			try {
			// execute current
				current.execute();
				current = current.succes;
			}
			catch(UnableToMoveException | ActionneurException e)
			{
				current = current.echec;
			}
		}
	}
	
	public Cinematique getPointEntree()
	{
		return pointEntree;
	}

}
