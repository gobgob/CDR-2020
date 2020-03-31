#pragma once

/* Conversion ticks-mm. Unité : mm/tick */
#define TICK_TO_MM              0.06506f

/* Conversion ticks-radians. Unité : radian/tick
 * (2 * TICK_TO_MM) / ECARTEMENT_CODEUSES */
#define TICK_TO_RADIANS         0.0006848f

/* Distance entre l'essieu avant et arrière */
#define WHEELBASE_LENGTH        135.0f  // mm

/* Nombre de valeurs à utiliser dans le calcul de la moyenne glissante
 * permettant de lisser la mesure de vitesse */
#define AVERAGE_SPEED_SIZE      50

/* Réglage des PWM et des ADC */
#define ANALOG_WRITE_FREQ       20000
#define ANALOG_WRITE_RES        11
#define ANALOG_READ_RES         10

/* Liaison série HL */
#define SERIAL_HL               Serial  // USB
#define SERIAL_HL_BAUDRATE      115200  // (ignoré par l'USB_Serial)

/* Liaison série AX12 */
#define SERIAL_AX12			    Serial2 // Pins 9 10
#define SERIAL_AX12_BAUDRATE    1000000
#define SERIAL_AX12_TIMEOUT     10      // ms (standard: 50ms, minimum: 2ms)

/* Liaison série ToF_Module */
#define SERIAL_TOF              Serial1 // Pins 0 1
#define SERIAL_TOF_BAUDRATE     200000
#define SERIAL_TOF_TIMEOUT      50

/* Liaison série extérieure */
#define SERIAL_EXT              Serial3 // Pins 7 8
#define SERIAL_EXT_BAUDRATE     200000
#define SERIAL_EXT_TIMEOUT      50

/* IDs des AX12 */
#define ID_AX12_DIRECTION   1
#define ID_AX12_HARPON_G    2
#define ID_AX12_HARPON_D    3
#define ID_AX12_TENTACLE_G  4
#define ID_AX12_TENTACLE_D  5

/* Adresses des périphériques I2C */
#define I2C_ADDR_7_SEGMENT  112

/* Encodeurs d'odométrie */
#define PIN_A_LEFT_ENCODER  34
#define PIN_B_LEFT_ENCODER  33
#define PIN_A_RIGHT_ENCODER 11
#define PIN_B_RIGHT_ENCODER 12

/* Moteur de propulsion */
#define PIN_MOT_INA         2
#define PIN_MOT_INB         3
#define PIN_MOT_PWM         4
#define PIN_MOT_SEL         5
#define PIN_MOT_CS          A3
#define PIN_ENC_MOT_A       14
#define PIN_ENC_MOT_B       15

/* Moteur du drapeau */
#define PIN_FLAG_INA        22
#define PIN_FLAG_INB        21
#define PIN_FLAG_PWM        20
#define PIN_FLAG_SEL        23
#define PIN_FLAG_CS         A2

/* Drapeau */
#define PIN_FLAG_GPIO1      31
#define PIN_FLAG_GPIO2      32

/* DELs */
#define PIN_LED_NEOPIXEL    24
#define PIN_LED_COLOR_A     35
#define PIN_LED_COLOR_B     36
#define PIN_LED_DEBUG_A     37
#define PIN_LED_DEBUG_B     38

/* Divers capteurs */
#define PIN_GET_COLOR_A     25  // Blue side
#define PIN_GET_COLOR_B     26  // Yellow side
#define PIN_GET_JUMPER      6
