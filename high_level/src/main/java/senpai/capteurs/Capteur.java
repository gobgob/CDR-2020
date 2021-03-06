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

package senpai.capteurs;

import java.awt.Graphics;

import pfg.kraken.display.Display;
import pfg.kraken.display.Printable;
import pfg.kraken.struct.Kinematic;
import pfg.kraken.struct.XY;
import pfg.kraken.struct.XY_RW;
import senpai.robot.Robot;

/**
 * Un capteur de proximité du robot
 * 
 * @author pf
 *
 */

public class Capteur implements Printable
{
	private static final long serialVersionUID = 1L;
	
	protected final XY positionRelative;
	protected final double orientationRelative;
	public final double angleCone; // angle du cône (en radians)
	private final int portee;
	protected double orientationRelativeRotate;
	protected XY_RW positionRelativeRotate;
	public TypeCapteur type;
	private Kinematic cinemRobot;

	public Capteur(Robot robot, XY positionRelative, double orientationRelative, TypeCapteur type)
	{
		cinemRobot = robot.getCinematique();
		this.type = type;
		this.positionRelative = positionRelative;
		this.orientationRelative = orientationRelative;
		positionRelativeRotate = new XY_RW();
		this.angleCone = type.angleCone;
		this.portee = type.portee;
		orientationRelativeRotate = orientationRelative;
		positionRelativeRotate = positionRelative.clone();

	}

	@Override
	public void print(Graphics g, Display f)
	{
		double orientation = cinemRobot.orientationReelle;
		XY_RW p1 = positionRelativeRotate.clone();
		p1.rotate(orientation);
		p1.plus(cinemRobot.getPosition());
		XY_RW p2 = positionRelativeRotate.clone();
		p2.plus(new XY(portee, angleCone + orientationRelativeRotate, false));
		p2.rotate(orientation);
		p2.plus(cinemRobot.getPosition());
		XY_RW p3 = positionRelativeRotate.clone();
		p3.plus(new XY(portee, -angleCone + orientationRelativeRotate, false));
		p3.rotate(orientation);
		p3.plus(cinemRobot.getPosition());
		int[] x = new int[3];
		x[0] = f.XtoWindow(p1.getX());
		x[1] = f.XtoWindow(p2.getX());
		x[2] = f.XtoWindow(p3.getX());
		int[] y = new int[3];
		y[0] = f.YtoWindow(p1.getY());
		y[1] = f.YtoWindow(p2.getY());
		y[2] = f.YtoWindow(p3.getY());
		g.setColor(type.couleurTransparente);
		g.fillPolygon(x, y, 3);
		g.setColor(type.couleur);
		g.drawPolygon(x, y, 3);
	}

	/**
	 * Utilisé pour l'affichage uniquement
	 * @param c
	 */
	public void setCinematique(Kinematic c)
	{
		cinemRobot = c.clone();
	}

}
