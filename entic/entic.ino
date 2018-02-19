
// PINS DEFINITION
#define PIN_JOY_SW 8

#define PIN_MOTOR_L_E 9 // Control Pin PWM ENABLE for Left Motor
#define PIN_MOTOR_L_DIR 7

#define PIN_MOTOR_R_E 11 // Control Pin PWM ENABLE for Right Motor
#define PIN_MOTOR_R_DIR 6

#define PIN_MOTOR_Z_E 10 // Control Pin PWM ENABLE for Z Motor
#define PIN_MOTOR_Z_DIR 5


#define PIN_JOY_X A1
#define PIN_JOY_Y A2
#define PIN_ACCEL A5

// CONSTANTS DEFINITION
#define PWM_50 127 // PWM pin signal range [0-255] 256b, 50% -> 127
#define PWM_100 255 // PWM pin signal range [0-255] 256b, 100% -> 255
#define ANALOG_50 511 // Analog pin signal received range [0-1023] 1024b, 50% -> 511

#define COUNT_FIRST_TAP 200
#define COUNT_DOUBLE_TAP 220

// LIMITS DEFINITION
//                    axis y
//                      ^
//          (Q2)        *  Y2       (Q1)
//                      *
//                      *  Y1
//                      *
// ***************************************>  axis x
// X_4, X_3, X_2, X_1   *  X1, X2, X3, X4
//                      *
//                      *  Y_1
//          (Q3)        *           (Q4)
//                      *  Y_2
//                      *

#define LIMIT_X_4 -480        // +Y 100% Motor R & -Y 100% Motor L
#define LIMIT_X_3 -380        // +Y 100% Motor R & -Y 50% Motor L
#define LIMIT_X_2 -280        // +Y 100% Motor R
#define LIMIT_X_1 -180        // +Y 50% Motor R

#define LIMIT_X1 180          // +Y 50% Motor L
#define LIMIT_X2 280          // +Y 100% Motor L
#define LIMIT_X3 380          // +Y 100% Motor L & -Y 50% Motor R
#define LIMIT_X4 480          // +Y 100% Motor L & -Y 100% Motor R

#define LIMIT_Y2 -480            // +Y 100% Motor (L & R)
#define LIMIT_Y1 -280            // +Y 50% Motor (L & R)

#define LIMIT_Y_1 280           // -Y 50% Motor (L & R)
#define LIMIT_Y_2 480           // -Y 100% Motor (L & R)

// Accel limits. Mid value (304-309)
#define LIMIT_ACCEL_Z_POS 400 // Accel up, correct deviation down
#define LIMIT_ACCEL_Z_NEG 200 // Accel down, correct deviation up

// Data from Joystic
int posX;
int posY;

// PWM enable motor.
int motorR;
int motorL;
int motorZ;

// Reverse or not mode for motors.
boolean reverseR;
boolean reverseL;
boolean reverseZ;

// Accelerometer Data
int accel;


void setup() {
  // initialize the serial port:
  Serial.begin(9600);

  pinMode(PIN_MOTOR_L_E, OUTPUT);
  pinMode(PIN_MOTOR_L_DIR, OUTPUT);
  pinMode(PIN_MOTOR_R_E, OUTPUT);
  pinMode(PIN_MOTOR_R_DIR, OUTPUT);
  pinMode(PIN_MOTOR_Z_E, OUTPUT);
  pinMode(PIN_MOTOR_Z_DIR, OUTPUT);
  pinMode(PIN_JOY_SW, INPUT);

  digitalWrite(PIN_MOTOR_L_E, LOW);
  digitalWrite(PIN_MOTOR_R_E, LOW);
  digitalWrite(PIN_MOTOR_Z_E, LOW);

  posX = 0;
  posY = 0;
  accel = 0;

  reverseR = LOW;
  reverseL = LOW;
  reverseZ = LOW;

}

void loop() {
  motorR = 0;
  motorL = 0;
  motorZ = 0;

  // If button is pull down, detect if button is pushed.
  if (!digitalRead(PIN_JOY_SW)) {
    int count;
    for (count = 0; count < COUNT_FIRST_TAP; count++) {
      // Waiting for button release... (if button is not release, count achieve COUNT_FIRST_TAP)
      if (digitalRead(PIN_JOY_SW)) {
        // Button released. Waiting for second button tap...
        for (int count1 = 0; count1 < 30; count1++) {
          if (!digitalRead(PIN_JOY_SW)) {
            // Double Button Tap.
            count = COUNT_DOUBLE_TAP;
            // Second tap detected. Interrupt counting.
            break;
          }
          delay(5); // 5 milisec * 200 = 1 second
        }
        // Button was released and no second tap detected.
        // Interrupt counting. ONLY ONE INSTANT TAP
        // RFU -> Reserved for Future Use
        break;
      }
      delay(5); // 5 milisec * 200 = 1 second
    }

    if (count == COUNT_FIRST_TAP) {
      reverseZ = LOW;
      updatePolarityMotors();
      // Waiting for button release...
      while(!digitalRead(PIN_JOY_SW)) {
        // Enable Z Motor
        digitalWrite(PIN_MOTOR_Z_E, HIGH);
      }
      // Disable Z Motor
      digitalWrite(PIN_MOTOR_Z_E, LOW);

    } else if (count == COUNT_DOUBLE_TAP) {
      reverseZ = HIGH;
      updatePolarityMotors();
      // Waiting for button release...
      while(!digitalRead(PIN_JOY_SW)) {
        // Enable Z Motor
        digitalWrite(PIN_MOTOR_Z_E, HIGH);
      }
      // Disable Z Motor
      digitalWrite(PIN_MOTOR_Z_E, LOW);
    } else {
      // ONLY ONE INSTANT TAP
      // RFU -> Reserved for Future Use
    }
  } else {
    accel = analogRead(PIN_ACCEL);
    if (accel > LIMIT_ACCEL_Z_POS) {
      reverseZ = LOW;
      digitalWrite(PIN_MOTOR_Z_E, HIGH);
    } else if (accel < LIMIT_ACCEL_Z_NEG) {
      reverseZ = HIGH;
      digitalWrite(PIN_MOTOR_Z_E, HIGH);
    } else {
      digitalWrite(PIN_MOTOR_Z_E, LOW);
    }
  }

  // read the sensor value:
  posX = analogRead(PIN_JOY_X) - 512;
  posY = analogRead(PIN_JOY_Y) - 512;

  if (posX > LIMIT_X4) {
    // +Y 100% Motor L & -Y 100% Motor R
    motorL = PWM_100;
    motorR = -PWM_100;

  } else if (posX > LIMIT_X3) {
    // +Y 100% Motor L & -Y 50% Motor R
    motorL = PWM_100;
    motorR = -PWM_50;

  } else if (posX > LIMIT_X2) {
    // +Y 100% Motor L
    motorL = PWM_100;

  } else if (posX > LIMIT_X1) {
    // +Y 50% Motor L
    motorL = PWM_50;

  } else if (posX < LIMIT_X_4) {
    // +Y 100% Motor R & -Y 100% Motor L
    motorL = -PWM_100;
    motorR = PWM_100;

  } else if (posX < LIMIT_X_3) {
    // +Y 100% Motor R & -Y 50% Motor L
    motorL = -PWM_50;
    motorR = PWM_100;

  } else if (posX < LIMIT_X_2) {
    // +Y 100% Motor R
    motorR = PWM_100;

  } else if (posX < LIMIT_X_1) {
    // +Y 50% Motor R
    motorR = PWM_50;

  } else {
    motorL = 0;
    motorR = 0;
  }

  if (posY < LIMIT_Y2) {
    // +Y 100% Motor (L & R)
    motorL += PWM_100;
    motorR += PWM_100;
  } else if (posY < LIMIT_Y1) {
    // +Y 50% Motor (L & R)
    motorL += PWM_50;
    motorR += PWM_50;
  } else if (posY > LIMIT_Y_2) {
    // -Y 100% Motor (L & R)
    motorL -= PWM_100;
    motorR -= PWM_100;
  } else if (posY > LIMIT_Y_1) {
    // -Y 50% Motor (L & R)
    motorL -= PWM_50;
    motorR -= PWM_50;
  }

  checkMotors();
  updatePolarityMotors();

  analogWrite(PIN_MOTOR_R_E, motorR);
  analogWrite(PIN_MOTOR_L_E, motorL);
}

void checkMotors() {
  // Check Left motor reverse mode.
  reverseL = LOW;
  if (motorL < 0) {
    reverseL = HIGH;
    motorL = -motorL;
  }

  // Check limit range Left motor.
  if (motorL > PWM_100) {
    motorL = PWM_100;
  }

  // Check Right motor reverse mode.
  reverseR = LOW;
  if (motorR < 0) {
    reverseR = HIGH;
    motorR = -motorR;
  }

  // Check limit range Right motor.
  if (motorR > PWM_100) {
    motorR = PWM_100;
  }
}

void updatePolarityMotors() {
  if (reverseR) {
    digitalWrite(PIN_MOTOR_R_DIR, LOW);
  } else {
    digitalWrite(PIN_MOTOR_R_DIR, HIGH);
  }

  if (reverseL) {
    digitalWrite(PIN_MOTOR_L_DIR, LOW);
  } else {
    digitalWrite(PIN_MOTOR_L_DIR, HIGH);
  }

  if (reverseZ) {
    digitalWrite(PIN_MOTOR_Z_DIR, LOW);
  } else {
    digitalWrite(PIN_MOTOR_Z_DIR, HIGH);
  }
}
