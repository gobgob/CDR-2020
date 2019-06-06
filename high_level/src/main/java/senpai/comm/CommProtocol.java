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

/**
 * Protocole série entre le bas niveau et la Java
 * 
 * @author pf
 *
 */

public class CommProtocol
{

	public enum State
	{
		OK,
		KO;
	}
	
	public enum Channel
	{
		DESINSCRIPTION,
		INSCRIPTION;
		
		public final byte code = (byte) ordinal();
		public final boolean started = ordinal() == 1;
	}

	public enum EtatCapteur
	{
		CAPTEUR_HS_BRULE_NOYE,
		PAS_MESURE,
		TROP_PROCHE,
		TROP_LOIN;
	}
	
	public enum Id
	{
		/**
		 * Protocole Java vers bas niveau
		 */
		
		// Canaux de données (0x00 à 0x1F)
		ODO_AND_SENSORS(0x00),
		

		// Ordres longs (0x20 à 0x7F)
		FOLLOW_TRAJECTORY(0x20),
		STOP(0x21, -20),
		WAIT_FOR_JUMPER(0x22),
		START_MATCH_CHRONO(0x23),
//		ACTUATOR_GO_HOME(0x24, "actuatorGoHome"), // gardé pour l'exemple
				
		// Ordres immédiats (0x80 à 0xFF)
		PING(0x80, true),
		ASK_COLOR(0x81, true),
		EDIT_POSITION(0x82, false),
		SET_POSITION(0x83, false),
		ADD_POINTS(0x84, true, -10),
		EDIT_POINTS(0x85, true, -10),
		DESTROY_POINTS(0x86, true, -10),
		SET_SCORE(0x87, false),
		SET_NIGHT_LIGHT(0x88, false),
		SET_WARNINGS(0x89, false),
		ACT_STOP(0x8A, false),
		ACT_GET_POSITION(0x8B, true),
		ENABLE_PARKING_BREAK(0x8C, false),
		ENABLE_HIGH_SPEED_MODE(0x8D, false),
		DISPLAY_COLOR(0x8E, false),
		
		DISPLAY(0x90, false),
		SAVE(0x91, false),
		LOAD_DEFAULTS(0x92, false),
		GET_POSITION(0x93, true),
		SET_CONTROL_LEVEL(0x94, false),
		START_MANUAL_MOVE(0x95, false),
		SET_MAX_SPEED(0x96, false),
		SET_DISTANCE_TO_DRIVE(0x97, false),
		SET_CURVATURE(0x98, false),
		SET_DIRECTION_ANGLE(0x99, false),
		TRANSLATION_CONSTANTS(0x9A, false),
		TRAJECTORY_CONSTANTS(0x9B, false),
		STOPPING_CONSTANTS(0x9C, false),
		SET_MAX_ACCELERATION(0x9D, false),
		SET_MAX_DECELERATION(0x9E, false),
		SET_MAX_CURVATURE(0x9F, false),
		SET_SMOKE_LEVEL(0xA0, false);

		// Paramètres constants
		public final byte code;
		public final int codeInt;
		private final boolean isLong; // ordre long ?
		private final boolean isStream; // stream ?
		private final boolean expectAnswer;
		public final int priority; // basse priorité = urgent
		private String name;
		
		// Variables d'état
		private volatile boolean waitingForAnswer; // pour les canaux, permet de savoir s'ils sont ouverts ou non
		private volatile boolean sendIsPossible; // "true" si un ordre long est lancé, "false" sinon
		private volatile long dateLastClose; // pour les streams qui peuvent mettre un peu de temps à s'éteindre
		
		private static final int maxTimeAfterClose = 10;
		
		// Ticket (nul si pas de réponse attendu)
		public final Ticket ticket;
		
		public final static Id[] LUT; // look-up table pour retrouver un ordre à partir de son code
		
		static {
			LUT = new Id[256];
			for(Id id : values())
				LUT[id.codeInt] = id;
		}
		
		public boolean isSendPossible()
		{
			return sendIsPossible;
		}
		
		public void changeStreamState(Channel newState)
		{
			assert isStream : "changeStreamState sur "+this+" qui n'est pas un canal de données (code = "+code+")";
//			assert waitingForAnswer != newState.started : newState.started ? "Canal déjà ouvert !" : "Canal déjà fermé !";
			waitingForAnswer = newState.started;
			if(!waitingForAnswer) // on vient de fermer le canal
				dateLastClose = System.currentTimeMillis();
		}
		
		public String getMethodName()
		{
			return name;
		}
		
		@Override
		public String toString()
		{
			return name()+" "+(isLong ? "LONG" : (isStream ? "STREAM" : "COURT"))+", état : waitingForAnswer="+waitingForAnswer+", sendIsPossible="+sendIsPossible;
		}
		
		// priorité par défaut : 0
		private Id(int code)
		{
			this(code, 0);
		}
	
		// priorité par défaut : 0
		private Id(int code, String name)
		{
			this(code, 0);
			this.name = name;
		}
		
		// constructeur des ordres longs et des streams
		private Id(int code, int priority)
		{
			// un ordre long (ou un stream) doit obligatoirement attendre une réponse
			this(code, true, priority);
			assert isLong || isStream;
		}
		
		// priorité par défaut : 0
		private Id(int code, boolean expectAnswer)
		{
			this(code, expectAnswer, 0);
		}
		
		private Id(int code, boolean expectAnswer, int priority)
		{
			assert code >= 0x00 &&code <= 0xFF : "ID interdit : "+code;;
			isStream = code <= 0x1F;
			isLong = code >= 0x20 && code < 0x80;
			this.priority = priority;
			this.expectAnswer = expectAnswer;
			// les streams doivent toujours pouvoir attendre une réponse
			assert (!isStream && !isLong) || expectAnswer : "Les canaux de données et les ordres longs doivent pouvoir attendre une réponse ! " + this;
			this.code = (byte) code;
			codeInt = code;
			sendIsPossible = true;
			waitingForAnswer = false;
			
			// Les canaux de données n'ont pas de tickets
			if(expectAnswer)
				ticket = new Ticket();
			else
				ticket = null;
			
			name = toString();
		}
		
		public void answerReceived()
		{
			// un stream peut mettre du temps à s'éteindre
			if(isStream && !waitingForAnswer)
				assert System.currentTimeMillis() - dateLastClose < maxTimeAfterClose : "Le stream "+this+" continue d'envoyer des messages après un désabonnement !";

			assert isStream || (waitingForAnswer && expectAnswer) : "Réponse inattendue reçue pour "+this+" ("+waitingForAnswer+", "+expectAnswer+")";
			if(!isStream) // si ce n'est pas un stream, on n'attend plus de nouvelles réponses
				waitingForAnswer = false;
			sendIsPossible = true; // ordres longs de nouveau envoyables
		}
		
		public void orderSent()
		{
			assert sendIsPossible : "Envoi impossible pour "+this+" : on attend encore une réponse";
			if(expectAnswer)
				waitingForAnswer = true;
			if(isLong) // si c'est un ordre long, on doit interdire l'envoi tant qu'on n'a pas reçu de réponse
				sendIsPossible = false;
		}
	}

	public enum ActionneurMask
	{
		STEPPER_BLOCKED,
		AX12_Y_BLOCKED,
		AX12_THETA_BLOCKED,
		AX12_ERR,
		MANUAL_STOP,
		UNREACHABLE,
		SENSOR_ERR,
		NO_DETECTION,
		MOVE_TIMED_OUT;

		public final int masque = 1 << ordinal();
		
		public static String describe(int valeur)
		{
			if(valeur == 0)
				return "";
			StringBuilder out = new StringBuilder();
			for(ActionneurMask m : values())
			{
				if((valeur & m.masque) != 0)
				{
					out.append(m.toString());
					out.append(" ");
				}
			}
			return out.toString();
		}

	}
	
	public enum TrajEndMask
	{
		STOP_REQUIRED,
		ROBOT_BLOCAGE_EXTERIEUR,
		ROBOT_BLOCAGE_INTERIEUR,
		TROP_LOIN,
		PLUS_DE_POINTS;
		
		private final int masque = 1 << ordinal();
		
		public static String describe(int valeur)
		{
			if(valeur == 0)
				return "";
			StringBuilder out = new StringBuilder();
			for(TrajEndMask m : values())
			{
				if((valeur & m.masque) != 0)
				{
					out.append(m.toString());
					out.append(" ");
				}
			}
			return out.toString();
		}

	}
	
	public enum LLStatus
	{
		/**
		 * Protocole bas niveau vers Java
		 */
		
		// Couleur
		COULEUR_VIOLET(0x00, State.OK),
		COULEUR_JAUNE(0x01, State.OK),
		COULEUR_ROBOT_INCONNU(0x02, State.KO),

		ACK_SUCCESS(0x00, State.OK),
		ACK_FAILURE(0x01, State.KO);

		public final int codeInt;
		public final State etat;

		private LLStatus(int code, State etat)
		{
			codeInt = code;
			this.etat = etat;
		}
	}

	public enum NightLightMode
	{
		OFF,
		LOW,
		MEDIUM,
		HIGH;
	}
	
}
