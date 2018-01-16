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


package senpai.buffer;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import pfg.graphic.log.Log;
import senpai.Severity;
import senpai.Subject;
import pfg.graphic.Chart;
import pfg.graphic.printable.Plottable;

public class IncomingBuffer<T> implements Plottable
{
	private static final long serialVersionUID = 1L;
	protected Log log;
	private boolean warning = false;
	private String nom;
	
	public IncomingBuffer(Log log, String nom)
	{
		this.log = log;
		this.nom = nom;
	}

	private BlockingQueue<T> buffer = new ArrayBlockingQueue<T>(200);

	/**
	 * Ajout d'un élément dans le buffer
	 * 
	 * @param elem
	 */
	public synchronized void add(T elem)
	{
		try {
			buffer.add(elem);
			if(buffer.size() > 20)
			{
				log.write("Buffer de "+elem.getClass().getSimpleName()+" traités trop lentement ! Taille buffer : " + buffer.size(), Severity.CRITICAL, Subject.COMM);
				warning = true;
			}
			else if(buffer.size() > 5)
			{
				log.write("Buffer de "+elem.getClass().getSimpleName()+" traités trop lentement ! Taille buffer : " + buffer.size(), Severity.WARNING, Subject.COMM);
				warning = true;
			}

		} catch(IllegalStateException e)
		{
			e.printStackTrace();
		}
	}
	
	/**
	 * Retire un élément du buffer
	 * 
	 * @return
	 * @throws InterruptedException 
	 */
	public synchronized T take() throws InterruptedException
	{
		T out = buffer.take();
		if(buffer.isEmpty() && warning)
		{
			log.write("Retard du buffer de "+out.getClass().getSimpleName()+" récupéré", Subject.COMM);
			warning = false;
		}
		return out;
	}

	@Override
	public void plot(Chart a)
	{
		a.addData(nom, (double) buffer.size());
	}
}