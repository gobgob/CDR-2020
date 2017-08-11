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

import exceptions.ActionneurException;
import exceptions.UnableToMoveException;

/**
 * Un nœud du graphe
 * @author pf
 *
 */

public class NodeActionne extends ScriptDAGNode
{

	public NodeActionne(ScriptDAGNode succes, ScriptDAGNode echec)
	{
		super(succes, echec);
	}

	@Override
	public void execute() throws InterruptedException, UnableToMoveException, ActionneurException
	{
		// bouge un bras, tout ça
	}

}
