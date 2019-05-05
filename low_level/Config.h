#ifndef CONFIG_H
#define CONFIG_H

/* TICK_TO_MM */
#define TICK_TO_MM				0.06506     // Conversion ticks-mm. Unité : mm/tick

/* TICK_TO_RADIANS
    (2 * TICK_TO_MM) / ECARTEMENT_CODEUSES
*/
#define TICK_TO_RADIANS			0.0006848   // Conversion ticks-radians. Unité : radian/tick

#define AVERAGE_SPEED_SIZE      50          // Nombre de valeurs à utiliser dans le calcul de la moyenne glissante permettant de lisser la mesure de vitesse


/* Liaison série AX12 */
#define SERIAL_AX12			    Serial1		// Pins 0 1
#define SERIAL_AX12_BAUDRATE    1000000
#define SERIAL_AX12_TIMEOUT     10          // ms  (standard is 50ms, minimum is 2ms)

/* IDs des AX12 */
#define ID_AX12_DIRECTION   2
#define ID_AX12_ACT_Y       0
#define ID_AX12_ACT_THETA   1

/* Adresses des périphériques I2C */
#define I2C_ADDR_7_SEGMENT          112
#define I2C_ADDR_TOF_AVG            42
#define I2C_ADDR_TOF_AVD            43
#define I2C_ADDR_TOF_FOURCHE_AVG    44
#define I2C_ADDR_TOF_FOURCHE_AVD    45
#define I2C_ADDR_TOF_FLAN_ARG       46
#define I2C_ADDR_TOF_FLAN_ARD       47
#define I2C_ADDR_TOF_ARG            48
#define I2C_ADDR_TOF_ARD            49

/* Encodeurs d'odométrie */
#define PIN_A_LEFT_ENCODER		34
#define PIN_B_LEFT_ENCODER		33
#define PIN_A_RIGHT_ENCODER		39
#define PIN_B_RIGHT_ENCODER		38

/* Moteur */
#define PIN_VESC            14

/* Moteurs pas à pas */
#define PIN_STEPPER_STEP    30
#define PIN_STEPPER_DIR     29
#define PIN_STEPPER_SLEEP   31
#define PIN_STEPPER_RESET   32
#define PIN_STEPPER_ENDSTOP 9
#define PIN_MICROSTEP_1     28
#define PIN_MICROSTEP_2     27
#define PIN_MICROSTEP_3     26

/* DELs */
#define PIN_DEL_WARNING	    5
#define PIN_DEL_ERROR	    4

/* Phares */
#define PIN_NEOPIXELS_FRONT 7
#define PIN_NEOPIXELS_BACK  6

/* Divers capteurs */
#define PIN_GET_COLOR		8
#define PIN_GET_JUMPER		37

/* Module Ethernet */
#define PIN_WIZ820_RESET	15
#define PIN_WIZ820_SS       10
#define PIN_WIZ820_MOSI     11
#define PIN_WIZ820_MISO     12
#define PIN_WIZ820_SCLK     13

/* ToF sensors */
#define PIN_EN_TOF_AVG          24
#define PIN_EN_TOF_AVD          35
#define PIN_EN_TOF_FOURCHE_AVG  25
#define PIN_EN_TOF_FOURCHE_AVD  36
#define PIN_EN_TOF_FLAN_ARG     21
#define PIN_EN_TOF_FLAN_ARD     16
#define PIN_EN_TOF_ARG          22
#define PIN_EN_TOF_ARD          23

/* Fumigène */
#define PIN_SMOKE_RESISTOR  20
#define PIN_SMOKE_PUMP      17

/* Extra */
#define PIN_EXTRA_1     2
#define PIN_EXTRA_2     3


#endif
