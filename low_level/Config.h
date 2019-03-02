#ifndef CONFIG_H
#define CONFIG_H

/* TICK_TO_MM
Th�orique : 0.0812
R�el, roues std : 0.0837
R�el, roues avec �lastiques : 0.0829
*/
#define TICK_TO_MM				0.0829      // Conversion ticks-mm pour les roues codeuses arri�res. Unit� : mm/tick

/* TICK_TO_RADIANS
Th�orique : 0.00108
R�el, roues std : 0.001101
R�el, roues avec �lastiques : ?
*/
#define TICK_TO_RADIANS			0.00108    // Conversion ticks-radians. Unit� : radian/tick

#define MOTOR_TICK_TO_TICK		2.4			// Conversion ticks_des_roues_avant --> ticks. Unit� : tick/ticks_des_roues_avant
#define AVERAGE_SPEED_SIZE      50          // Nombre de valeurs � utiliser dans le calcul de la moyenne glissante permettant de lisser la mesure de vitesse


/* D�finition des dimensions physiques du robot, en mm */

// Ecartement des roues avant et les codeuses
#define FRONT_BACK_WHEELS_DISTANCE	90

// Position du centre de rotation du bloc de direction, en mm
#define DIRECTION_ROTATION_POINT_Y	52

// Distance s�parant le centre de rotation du bloc direction de la roue associ�e, en mm
#define DIRECTION_WHEEL_DIST_FROM_ROT_PT	28



/* Encodeurs d'odom�trie */
#define PIN_A_LEFT_ENCODER		16
#define PIN_B_LEFT_ENCODER		17
#define PIN_A_RIGHT_ENCODER		18
#define PIN_B_RIGHT_ENCODER		39


/* Encodeurs des moteurs de propultion */
#define PIN_A_FRONT_LEFT_MOTOR_ENCODER		24
#define PIN_B_FRONT_LEFT_MOTOR_ENCODER		25
#define PIN_A_FRONT_RIGHT_MOTOR_ENCODER		26
#define PIN_B_FRONT_RIGHT_MOTOR_ENCODER		27
#define PIN_A_BACK_LEFT_MOTOR_ENCODER		28
#define PIN_B_BACK_LEFT_MOTOR_ENCODER		29
#define PIN_A_BACK_RIGHT_MOTOR_ENCODER		30
#define PIN_B_BACK_RIGHT_MOTOR_ENCODER		38


/* Ponts en H des moteurs de propultion */
#define PIN_EN_FRONT_LEFT_MOTOR		2
#define PIN_A_FRONT_LEFT_MOTOR		3
#define PIN_B_FRONT_LEFT_MOTOR		4
#define PIN_EN_FRONT_RIGHT_MOTOR	5
#define PIN_A_FRONT_RIGHT_MOTOR		6
#define PIN_B_FRONT_RIGHT_MOTOR		9
#define PIN_EN_BACK_LEFT_MOTOR		21
#define PIN_A_BACK_LEFT_MOTOR		22
#define PIN_B_BACK_LEFT_MOTOR		23
#define PIN_EN_BACK_RIGHT_MOTOR		35
#define PIN_A_BACK_RIGHT_MOTOR		36
#define PIN_B_BACK_RIGHT_MOTOR		37


/* DELs */
#define PIN_DEL_STATUS_1	14
#define PIN_DEL_STATUS_2	20


/* Divers capteurs */
#define PIN_GET_COLOR		A22
#define PIN_GET_VOLTAGE		A21
#define PIN_GET_JUMPER		19


/* Module Ethernet */
#define PIN_WIZ820_RESET	15
#define PIN_WIZ820_SS       10
#define PIN_WIZ820_MOSI     11
#define PIN_WIZ820_MISO     12
#define PIN_WIZ820_SCLK     13


/* Liaisons s�rie */
#define SERIAL_ACTUATOR		Serial1		// Pins 0 1
#define SERIAL_SENSORS		Serial5		// Pins 33 34
#define SERIAL_AX12			Serial3		// Pins 7 8
#define SERIAL_XL320		Serial4	    // Pins 31 32

/* D�bits */
#define SERIAL_ACTUATOR_BAUDRATE    115200
#define SERIAL_SENSORS_BAUDRATE     115200
#define SERIAL_AX12_BAUDRATE        1000000
#define SERIAL_XL320_BAUDRATE       1000000

/* Timeout */
#define SERIAL_AX12_TIMEOUT         50  // ms  (standard is 50ms, minimum is 2ms)



/* Sensors config */
#define NB_SENSORS      9


/* ToF sensors */
#define PIN_EN_TOF_AVG      2
#define PIN_EN_TOF_AV       7
#define PIN_EN_TOF_AVD      8
#define PIN_EN_TOF_FLAN_AVG 17
#define PIN_EN_TOF_FLAN_AVD 11
#define PIN_EN_TOF_FLAN_ARG 16
#define PIN_EN_TOF_FLAN_ARD 12
#define PIN_EN_TOF_ARG      15
#define PIN_EN_TOF_ARD      14


#endif
