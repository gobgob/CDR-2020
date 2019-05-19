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

package senpai.table;

import java.awt.Color;
import java.awt.Graphics;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import pfg.config.Config;
import pfg.graphic.GraphicDisplay;
import pfg.graphic.GraphicPanel;
import pfg.graphic.printable.Layer;
import pfg.graphic.printable.Printable;
import pfg.kraken.obstacles.Obstacle;
import pfg.log.Log;
import senpai.utils.ConfigInfoSenpai;
import senpai.utils.Subject;

/**
 * Gère les éléments de jeux
 * 
 * @author pf
 *
 */

public class Table implements Printable
{
	private static final long serialVersionUID = 1L;

	// Dépendances
	protected transient Log log;

	private HashMap<AtomeParTerre, Boolean> etat = new HashMap<AtomeParTerre, Boolean>();
	private List<Obstacle> currentObstacles = new ArrayList<Obstacle>();
	private List<Obstacle> otherObstacles = new ArrayList<Obstacle>();
	
	public Table(Log log, Config config, GraphicDisplay buffer)
	{
		this.log = log;
		if(config.getBoolean(ConfigInfoSenpai.GRAPHIC_ENABLE))
			buffer.addPrintable(this, Color.BLACK, Layer.BACKGROUND.layer);
		for(AtomeParTerre n : AtomeParTerre.values())
			etat.put(n, false);
	}
	
	public void addOtherObstacle(Obstacle o)
	{
		otherObstacles.add(o);
	}
	
	/**
	 * On a pris l'objet, on est passé dessus, le robot ennemi est passé
	 * dessus...
	 * Attention, on ne peut qu'augmenter cette valeur.
	 * 
	 * @param id
	 */
	public void setDone(AtomeParTerre id)
	{
		log.write("Atome absent de la table : "+id, Subject.TABLE);
		etat.put(id, true);
	}

	/**
	 * Cet objet est-il présent ou non?
	 * 
	 * @param id
	 */
	public boolean isDone(AtomeParTerre id)
	{
		return etat.get(id);
	}

	public Iterator<Obstacle> getCurrentObstaclesIterator()
	{
		currentObstacles.clear();
		for(AtomeParTerre n : AtomeParTerre.values())
			if(!etat.get(n))
				currentObstacles.add(n.obstacle);
		currentObstacles.addAll(otherObstacles);
		return currentObstacles.iterator();
	}
	
	@Override
	public void print(Graphics g, GraphicPanel f)
	{
	}
}