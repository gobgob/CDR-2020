#include <Dynamixel.h>
#include <DynamixelInterface.h>
#include <DynamixelMotor.h>

#define STEP 30
#define DIR 29
#define SLEEP 31
#define RESET 32
#define MS1 28
#define MS2 27
#define MS3 26

#define ENDSTOP 9

#define LED_ERROR 4
#define LED_WARN 5

#define VERSION "0.1.1"

#define ABSOLUTE 0
#define INCREMENTAL 1

char serialInput[10];
int serialPrompt = 0;

DynamixelInterface dInterface(Serial1);  // Stream
DynamixelMotor motorY(dInterface, 0);  // Interface, ID;
DynamixelMotor motorA(dInterface, 1); // Interface, ID;

int currentMotorYPosition;
int currentMotorAPosition;
float currentMotorZPosition;
int speed = 80;
int mode = ABSOLUTE;

void setup() {
  // AX-12
  dInterface.begin(1000000, 50); // baudrate, timeout

  motorY.init(); // This will get the returnStatusLevel of the servo
  Serial.printf("[Motor Y] Status return level = %u\n", motorY.statusReturnLevel());
  motorY.jointMode(); // Set the angular limits of the servo. Set to [min, max] by default (85, 215)
  motorY.enableTorque();

  motorA.init(); // This will get the returnStatusLevel of the servo
  Serial.printf("[Motor A] Status return level = %u\n", motorA.statusReturnLevel());
  motorA.jointMode(); // Set the angular limits of the servo. Set to [min, max] by default (85, 215)
  motorA.enableTorque();

  // Steppers
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(SLEEP, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);

  // wakeup
  digitalWrite(RESET, HIGH);
  digitalWrite(SLEEP, HIGH);

  // set microsteps to 16
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);

  // Endstop
  pinMode(ENDSTOP, INPUT);

  // leds
  pinMode(LED_ERROR, OUTPUT);
  pinMode(LED_WARN, OUTPUT);

  help();
  init();
  ready();
}


void loop()
{
  readSerial();
}

/**
   Setup the Z
*/
void init() {
  digitalWrite(LED_WARN, HIGH);
  digitalWrite(LED_ERROR, HIGH);
  Serial.println("[MotorZ] Calibration start");
  digitalWrite(DIR, HIGH);
  while (digitalRead(ENDSTOP) == LOW) {
    digitalWrite(STEP, HIGH);
    delayMicroseconds(speed);
    digitalWrite(STEP, LOW);
    delayMicroseconds(speed);
  }
  currentMotorZPosition = 208; // Top
  Serial.println("[MotorZ] Calibration finish");
  home();
  digitalWrite(LED_WARN, LOW);
}

/**
   Display helpful information
*/
void help() {
  Serial.print(F("Atome manipulator "));
  Serial.println(VERSION);
  Serial.println(F("Commands:"));
  Serial.println(F("G0 [Y(steps)] [Z(steps)] [A(steps)] [F(feedrate)]; - linear move"));
  Serial.println(F("Y(steps); - Move Y axis"));
  Serial.println(F("Z(steps); - Move Z axis"));
  Serial.println(F("A(steps); - Move A axis"));
  Serial.println(F("G90; - absolute mode (default)"));
  Serial.println(F("G91; - incremental mode"));
  Serial.println(F("G28; - go home"));
  Serial.println(F("G29; - go middle"));
  Serial.println(F("G50; - disable torque"));
  Serial.println(F("G51; - enable torque"));
  Serial.println(F("M1; - init"));
  Serial.println(F("M100; - this help message"));
  Serial.println(F("M113; - report feedback position (ax-12 values)"));
  Serial.println(F("M114; - report position"));
  Serial.println(F("M115; - get firmware version"));
}

/**
   Read serial inputs
*/
void readSerial() {
  if (!Serial.available()) return;
  char c = Serial.read();
  serialInput[serialPrompt++] = c;

  if (c == '\n') {
    Serial.print(F("\n\r"));
    serialInput[serialPrompt] = 0; // strings must end with a \0
    processCommand();
    ready();
  }
}

/**
   Prepares the input buffer to receive a new message and
   tells the serial connected device it is ready for more.
*/
void ready() {
  serialPrompt = 0;
  Serial.print(F("> "));
}

void processCommand() {
  // look for command starting with "G"
  int cmd = parseNumber('G', -1);

  switch (cmd) {
    case 0: // move
      {
        int Y = parseNumber('Y', 0);
        int Z = parseNumber('Z', 0);
        int A = parseNumber('A', 0);
        int F = parseNumber('F', 0);

        if (F != 0) setFeedRate(F);
        if (A != 0) mode == INCREMENTAL ? motorAGotoInc(A) : motorAGotoAbs(A);
        if (Y != 0) mode == INCREMENTAL ? motorYGotoInc(Y) : motorYGotoAbs(Y);
        if (Z != 0) mode == INCREMENTAL ? motorZGotoInc(Z) : motorZGotoAbs(Z);
        break;
      }

    case 28: home(); break;
    case 29: {
        setFeedRate(40);
        motorYGotoAbs(150);
        motorAGotoAbs(150);
        motorZGotoAbs(140);
        break;
      }
    case 50: {
      motorY.enableTorque(false);
      motorA.enableTorque(false);
      break;
    }
    case 51: {
      motorY.enableTorque(true);
      motorA.enableTorque(true);
      break;
    }

    case 90: mode = ABSOLUTE; break;
    case 91: mode = INCREMENTAL; break;
  }

  // look for command starting with "M"
  cmd = parseNumber('M', -1);

  switch (cmd) {
    case 1: init(); break;
    case 100: help(); break;
    case 113: reportFeedbackPosition(); break;
    case 114: reportPosition(); break;
    case 115:
      Serial.printf("ok PROTOCOL_VERSION:%s FIRMWARE_NAME:AtomeManipulator\n", VERSION);
      break;
  }

  // X axis shortcut
  cmd = parseNumber('Y', 0);
  if (cmd != 0) mode == INCREMENTAL ? motorYGotoInc(cmd) : motorYGotoAbs(cmd);

  // Y axis shortcut
  cmd = parseNumber('Z', 0);
  if (cmd != 0) mode == INCREMENTAL ? motorZGotoInc(cmd) : motorZGotoAbs(cmd);

  // A axis shortcut
  cmd = parseNumber('A', 0);
  if (cmd != 0) mode == INCREMENTAL ? motorAGotoInc(cmd) : motorAGotoAbs(cmd);
}

/**
   Set Y speed, 40 is the faster
*/
void setFeedRate(int i) {
  if (i < 40 || i > 500) {
    Serial.println("[FeedRate] Range Error (40 to 500)");
  } else {
    speed = i;
  }
}

void motorAGotoAbs(int pos) {
  if (pos < 60 || pos > 190) {
    Serial.println("[Motor A] Range Error (60 to 190)");
  } else {
    Serial.print("[Motor A] goto: "); Serial.println(pos, DEC);
    motorA.goalPositionDegree(pos);
    currentMotorAPosition = pos;
  }
}

void motorAGotoInc(int pos) {
  motorAGotoAbs(currentMotorAPosition + pos);
}

void motorYGotoAbs(int pos) {
  if (pos < 85 || pos > 215) {
    Serial.println("[Motor Y] Range Error (85 to 215)");
  } else {
    Serial.print("[Motor Y] goto: "); Serial.println(pos, DEC);
    motorY.goalPositionDegree(pos);
    currentMotorYPosition = pos;
  }
}

void motorYGotoInc(int pos) {
  motorYGotoAbs(currentMotorYPosition + pos);
}

/**
   Move Z axis of `pos` mm (incremental)
*/
void motorZGotoInc(int pos) {
  if (currentMotorZPosition + pos < 0 || currentMotorZPosition + pos > 208) {
    Serial.println("[Motor Z] Range Error (0 to 208)");
  } else {
    // Direction
    digitalWrite(DIR, pos > 0 ? HIGH : LOW);

    float mmPerStep = 0.04 / 16; // 0.04 mm per step | 16 microsteps
    float goal = currentMotorZPosition + pos;

    while (fabs(currentMotorZPosition - goal) > 0.01) {
      digitalWrite(STEP, HIGH);
      delayMicroseconds(speed);
      digitalWrite(STEP, LOW);
      delayMicroseconds(speed);
      currentMotorZPosition += (digitalRead(DIR) == HIGH ? mmPerStep : -mmPerStep);
    }
  }
}

/**
   Move Z axis of `pos` mm (absolute)
*/
void motorZGotoAbs(int pos) {
  if (pos < 0 || pos > 205) {
    // Abs have less max value due to the endstop
    Serial.println("[Motor Z] Range Error (0 to 205)");
  } else {
    // Direction
    digitalWrite(DIR, pos - currentMotorZPosition > 0 ? HIGH : LOW);

    float mmPerStep = 0.04 / 16; // 0.04 mm per step | 16 microsteps

    while (fabs(currentMotorZPosition - pos) > 0.01) {
      if (digitalRead(ENDSTOP) == HIGH) break;
      digitalWrite(STEP, HIGH);
      delayMicroseconds(speed);
      digitalWrite(STEP, LOW);
      delayMicroseconds(speed);
      currentMotorZPosition += (digitalRead(DIR) == HIGH ? mmPerStep : -mmPerStep);
    }
  }

}

void reportPosition() {
  Serial.print("[Motor Y]: position = "); Serial.println(currentMotorYPosition, DEC);
  Serial.print("[Motor Z]: position = "); Serial.println(currentMotorZPosition, DEC);
  Serial.print("[Motor A]: position = "); Serial.println(currentMotorAPosition, DEC);
  Serial.print("[FeedRate]: speed = "); Serial.println(speed, DEC);
}

void reportFeedbackPosition() {
  uint16_t yAngle;
  uint16_t aAngle;
  motorY.currentPositionDegree(yAngle);
  motorA.currentPositionDegree(aAngle);

  Serial.print("[Motor Y]: position = "); Serial.println(yAngle, DEC);
  Serial.print("[Motor A]: position = "); Serial.println(aAngle, DEC);
}

void home() {
  Serial.println("Go home");
  setFeedRate(40);
  digitalRead(ENDSTOP) == HIGH ? motorZGotoInc(-20) : motorZGotoAbs(208 - 20);
  motorYGotoAbs(150);
  motorAGotoAbs(60);
}

/**
   Look for character /code/ in the buffer and read the float that immediately follows it.
   @return the value found.  If nothing is found, /val/ is returned.
   @input code the character to look for.
   @input val the return value if /code/ is not found.
 **/
float parseNumber(char code, float val) {
  char *ptr = serialInput; // start at the beginning of buffer
  while ((long)ptr > 1 && (*ptr) && (long)ptr < (long)serialInput + serialPrompt) { // walk to the end
    if (*ptr == code) { // if you find code on your walk,
      return atof(ptr + 1); // convert the digits that follow into a float and return it
    }
    ptr = strchr(ptr, ' ') + 1; // take a step from here to the letter after the next space
  }
  return val;  // end reached, nothing found, return default val.
}
