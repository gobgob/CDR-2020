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

package senpai.exceptions;

/**
 * Exception levée lors d'un problème d'actionneur
 * 
 * @author pf
 *
 */

public class ActionneurException extends Exception
{

	private static final long serialVersionUID = -960091158805232282L;
	public final int code;
	
	public ActionneurException(String m, int code)
	{
		super(m);
		this.code = code;
	}

	public ActionneurException(String m, Throwable e, int code)
	{
		super(m, e);
		this.code = code;
	}	

}
