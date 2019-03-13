#ifndef CONFIG_H
#define CONFIG_H

/* TICK_TO_MM */
#define TICK_TO_MM				0.06506     // Conversion ticks-mm. Unité : mm/tick

/* TICK_TO_RADIANS
    (2 * TICK_TO_MM) / ECARTEMENT_CODEUSES
*/
#define TICK_TO_RADIANS			0.0006848   // Conversion ticks-radians. Unité : radian/tick

#define AVERAGE_SPEED_SIZE      50          // Nombre de valeurs à utiliser dans le calcul de la moyenne glissante permettant de lisser la mesure de vitesse


/* IDs des AX12 */
#define ID_AX12_DIRECTION  0


/* Encodeurs d'odométrie */
#define PIN_A_LEFT_ENCODER		34
#define PIN_B_LEFT_ENCODER		33
#define PIN_A_RIGHT_ENCODER		39
#define PIN_B_RIGHT_ENCODER		38

/* Moteur */
#define PIN_VESC            14

/* DELs */
#define PIN_DEL_STATUS_1	5
#define PIN_DEL_STATUS_2	4

/* Divers capteurs */
#define PIN_GET_COLOR		8
#define PIN_GET_JUMPER		37

/* Module Ethernet */
#define PIN_WIZ820_RESET	15
#define PIN_WIZ820_SS       10
#define PIN_WIZ820_MOSI     11
#define PIN_WIZ820_MISO     12
#define PIN_WIZ820_SCLK     13

/* Liaison série */
#define SERIAL_AX12			Serial1		// Pins 0 1
#define SERIAL_AX12_BAUDRATE        1000000

/* Timeout */
#define SERIAL_AX12_TIMEOUT     50  // ms  (standard is 50ms, minimum is 2ms)

/* ToF sensors */
#define PIN_EN_TOF_AVG          24
#define PIN_EN_TOF_AVD          35
#define PIN_EN_TOF_FOURCHE_AVG  25
#define PIN_EN_TOF_FOURCHE_AVD  36
#define PIN_EN_TOF_FLAN_ARG     21
#define PIN_EN_TOF_FLAN_ARD     16
#define PIN_EN_TOF_ARG          22
#define PIN_EN_TOF_ARD          23


#endif
