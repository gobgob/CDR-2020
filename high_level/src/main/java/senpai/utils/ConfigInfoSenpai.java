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

package senpai.utils;

import java.util.ArrayList;
import java.util.List;

import pfg.config.ConfigInfo;

/**
 * La config du robot
 * @author pf
 *
 */

public enum ConfigInfoSenpai implements ConfigInfo
{
	CHECK_LATENCY(false), // estime la latence de la communication
	DISABLE_JUMPER(false),
	ENABLE_LIDAR(false),
	RUSH_SPEED(0.8),
	ENABLE_PARKING_BREAK(true),
	
	/**
	 * Position initiale du robot
	 */
	INITIAL_X(1350),
	INITIAL_Y(1200),
	INITIAL_O(Math.PI/2),

	DEFAULT_MAX_SPEED(0.8),
	MAX_SPEED_IN_ENEMY(0.4),
	SLOW_OBSTACLE_RADIUS_MULTIPLIER(1.5),
	LIDAR_OBSTACLE_RADIUS(100),
//	TIMEOUT_LIDAR(2000),
//	LIDAR_COMMAND("/home/pi/lidar-processor/lidar_env/bin/python /home/pi/lidar-processor/src/main_script.py"),
//	ENABLE_LIDAR_SCRIPT(false),
	
	/**
	 * Infos sur le robot
	 */
	
	// par "non-déployé", comprendre "forme du robot en mouvement"
	// utilisé par le pathfinding
	DEMI_LONGUEUR_NON_DEPLOYE_ARRIERE(240), // distance entre le centre du robot
											// et le bord arrière du robot
											// non-déployé
	DEMI_LONGUEUR_DEPLOYE_AVANT(83), // distance entre le centre du
												// robot et le bord avant du
												// robot non-déployé
	DEMI_LONGUEUR_NON_DEPLOYE_AVANT(83), // distance entre le centre du
	// robot et le bord avant du
	// robot déployé
	LARGEUR_NON_DEPLOYE(176), // distance entre le bord gauche et le bord droit
								// du robot non-déployé
	MARGE_PATHFINDING(20), // marge latérale sur la dimension du robot

	/**
	 * Paramètres du pathfinding
	 */
	ALLOW_PRECOMPUTED_PATH(false), // autorise-t-on l'utilisation de chemins
									// précalculés. Si le robot fonctionne bien sans, autant désactiver
	ENABLE_KNOWN_PATHS(false), // active les chemins enregistrés ?

	/**
	 * Paramètres de la comm
	 */
	ETH_LL_PORT_NUMBER(2020), // port socket LL
	ETH_LL_HOSTNAME_SERVER("127.0.0.1"), // adresse ip du LL. Un hostname fonctionne aussi
	
	ETH_LIDAR_PORT_NUMBER(8765), // port socket HL
	ETH_ELECTRON_PORT_NUMBER(8766), // port socket HL
	/**
	 * Paramètres du traitement des capteurs
	 */
	DISTANCE_MAX_ENTRE_MESURE_ET_OBJET(50), // quelle marge d'erreur
											// autorise-t-on entre un objet et
											// sa détection
	DISTANCE_MAX_BORDURE(150), // distance max où on peut voir un objet près du bord
	TAILLE_BUFFER_RECALAGE(50), // combien de mesures sont nécessaires pour
								// obtenir une correction de recalage
	PEREMPTION_CORRECTION(100), // temps maximal entre deux mesures de
								// correction au sein d'un même buffer (en ms)
	ENABLE_DYNAMIC_CORRECTION(false), // la correction de position et d'orientation
								// est-elle activée ?
	WARM_UP_DURATION(5000), // durée du warm-up
	
	/**
	 * Log
	 */
	PRINT_STATUS(true),
	PRINT_CAPTEURS(false),
	PRINT_CORRECTION(true),
	PRINT_COMM(false),
	PRINT_TRAJECTORY(true),
	PRINT_SCRIPT(true),
	PRINT_TABLE(true),
	PRINT_WARNING(false),
	PRINT_CRITICAL(true),
	
	/**
	 * Debug
	 */
//	NO_OBSTACLES(false), // désactive tous les obstacles. Utile pour debug
	SIMULE_COMM(false), // la comm doit-elle être simulée (utile pour debug du HL)

	/**
	 * Interface graphique
	 */
//	GRAPHIC_HEURISTIQUE(false), // affichage des orientations heuristiques
								// données par le D* Lite
	GRAPHIC_ENABLE(false), // désactive tout affichage si faux (empêche le
							// thread d'affichage de se lancer)
	GRAPHIC_ROBOT_PATH("/navire.png"), // image du robot sans les
													// roues
	GRAPHIC_SEEN_OBSTACLES(false), // affiche les obstacles vus
	GRAPHIC_ROBOT_AND_SENSORS(false), // affiche le robot et ses capteurs
	GRAPHIC_TRACE_ROBOT(false), // affiche la trace du robot
	GRAPHIC_PATH(false); // affiche le chemin en cours


	private Object defaultValue;
	public boolean overridden = false;
	public volatile boolean uptodate;

	public static List<ConfigInfo> getGraphicConfigInfo()
	{
		List<ConfigInfo> out = new ArrayList<ConfigInfo>();
		for(ConfigInfoSenpai c : values())
			if(c.toString().startsWith("GRAPHIC_"))
				out.add(c);
		return out;
	}
	
	/**
	 * Par défaut, une valeur est constante
	 * 
	 * @param defaultValue
	 */
	private ConfigInfoSenpai(Object defaultValue)
	{
		this.defaultValue = defaultValue;
	}

	public Object getDefaultValue()
	{
		return defaultValue;
	}
}
