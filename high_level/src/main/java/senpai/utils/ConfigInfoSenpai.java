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
//	AFFICHAGE_TIMEOUT(5000), // timeout sur l'affichage (0 pour infini)
	CHECK_LATENCY(false), // estime la latence de la communication
	DISABLE_JUMPER(false),
	ENABLE_LIDAR(false),
	RUSH_SPEED(0.8),
	
	/**
	 * Position initiale du robot
	 */
	INITIAL_X(1210),
	INITIAL_Y(1400),
	INITIAL_O(Math.PI),

	DEFAULT_MAX_SPEED(0.8),
	MAX_SPEED_IN_ENEMY(0.4),
	SLOW_OBSTACLE_RADIUS_MULTIPLIER(1.5),
	/**
	 * Infos sur le robot
	 */
	
	// par "non-déployé", comprendre "forme du robot en mouvement"
	// utilisé par le pathfinding
	DEMI_LONGUEUR_NON_DEPLOYE_ARRIERE(278), // distance entre le centre du robot
											// et le bord arrière du robot
											// non-déployé
	DEMI_LONGUEUR_DEPLOYE_AVANT(263), // distance entre le centre du
												// robot et le bord avant du
												// robot non-déployé
	DEMI_LONGUEUR_NON_DEPLOYE_AVANT(106), // distance entre le centre du
	// robot et le bord avant du
	// robot déployé
	LARGEUR_NON_DEPLOYE(210), // distance entre le bord gauche et le bord droit
								// du robot non-déployé
	MARGE_PATHFINDING(20), // marge latérale sur la dimension du robot

	/**
	 * Paramètre de comportement / de scripts
	 */
	TAILLE_CARGO_MAX(5),

	/**
	 * Paramètres du pathfinding
	 */
	ALLOW_PRECOMPUTED_PATH(false), // autorise-t-on l'utilisation de chemins
									// précalculés. Si le robot fonctionne bien sans, autant désactiver
	ENABLE_KNOWN_PATHS(false), // active les chemins enregistrés ?

	/**
	 * Paramètres de la comm
	 */
	COMM_MEDIUM("Ethernet"),
	
	ETH_LL_PORT_NUMBER(80), // port socket LL
	ETH_LL_HOSTNAME_SERVER("172.16.0.2"), // adresse ip du LL. Un hostname fonctionne aussi
	
	ETH_HL_HOSTNAME_SERVER("127.0.0.1"),
	ETH_LIDAR_PORT_NUMBER(8765), // port socket HL
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
	PRINT_CORRECTION(false),
	PRINT_COMM(false),
	PRINT_TRAJECTORY(true),
	PRINT_SCRIPT(true),
	PRINT_TABLE(true),
	PRINT_WARNING(true),
	PRINT_CRITICAL(true),
	
	/**
	 * Debug
	 */
	NO_OBSTACLES(false), // désactive tous les obstacles. Utile pour debug
	SIMULE_COMM(false), // la comm doit-elle être simulée (utile pour debug du HL)

	/**
	 * Interface graphique
	 */
//	GRAPHIC_HEURISTIQUE(false), // affichage des orientations heuristiques
								// données par le D* Lite
	GRAPHIC_ENABLE(false), // désactive tout affichage si faux (empêche le
							// thread d'affichage de se lancer)
//	GRAPHIC_D_STAR_LITE(false), // affiche les calculs du D* Lite
//	GRAPHIC_D_STAR_LITE_FINAL(false), // affiche l'itinéraire final du D* Lite
//	GRAPHIC_PROXIMITY_OBSTACLES(true), // affiche les obstacles de proximité
//	GRAPHIC_TRAJECTORY(false), // affiche les trajectoires temporaires
//	GRAPHIC_TRAJECTORY_ALL(false), // affiche TOUTES les trajectoires
									// temporaires
//	GRAPHIC_TRAJECTORY_FINAL(true), // affiche les trajectoires
//	GRAPHIC_FIXED_OBSTACLES(true), // affiche les obstacles fixes
//	GRAPHIC_GAME_ELEMENTS(true), // affiche les éléments de jeux
//	GRAPHIC_ROBOT_COLLISION(false), // affiche les obstacles du robot lors de la
									// vérification des collisions
	GRAPHIC_ROBOT_PATH("/camion.png"), // image du robot sans les
													// roues
	GRAPHIC_SEEN_OBSTACLES(false), // affiche les obstacles vus
	GRAPHIC_ROBOT_AND_SENSORS(false), // affiche le robot et ses capteurs
	GRAPHIC_TRACE_ROBOT(false), // affiche la trace du robot
	GRAPHIC_PATH(false), // affiche le chemin en cours
	GRAPHIC_COMM_CHART(false), // active les graphes de debug de la communication
	GRAPHIC_CAPTEURS_CHART(false); // active les graphes de debug des capteurs


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
