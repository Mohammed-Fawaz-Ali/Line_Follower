/*
 * ============================================================
 *  Line Follower Robot — Arduino Nano
 * ============================================================
 *  IR Logic  : LOW  = sensor over BLACK line
 *              HIGH = sensor over WHITE surface
 *  Track     : White board, black line
 *
 *  Motor driver: L293D (DIP-16)
 *
 *  ⚠ NOTE: L293D is NOT recommended for motor projects.
 *    It has ~2.8V total voltage drop across both H-bridge
 *    stages, meaning motors receive less voltage than supply.
 *    It is also limited to 600mA per channel.
 *    For future builds, use TB6612FNG or L298N instead.
 *    This build uses L293D and works fine for light N20 motors.
 *
 *  Pin Map:
 *    IR Left   → D6
 *    IR Right  → D7
 *    L293D IN1 → D8   (Left  motor, direction A)
 *    L293D IN2 → D9   (Left  motor, direction B / PWM)
 *    L293D IN3 → D10  (Right motor, direction A / PWM)
 *    L293D IN4 → D11  (Right motor, direction B / PWM)
 *
 *  Author  : <your name>
 *  Version : 1.0.0
 *  License : MIT
 * ============================================================
 */

// ─── Pin Definitions ────────────────────────────────────────
#define IR_LEFT    6    // Left  IR sensor output
#define IR_RIGHT   7    // Right IR sensor output

#define L_IN1      8    // Left  motor — direction pin A
#define L_IN2      9    // Left  motor — direction pin B (PWM)
#define R_IN1     10    // Right motor — direction pin A (PWM)
#define R_IN2     11    // Right motor — direction pin B (PWM)

// ─── Speed Settings ─────────────────────────────────────────
// Range 0–255. Lower = slower but more stable on sharp turns.
#define SPEED_FWD    180   // Forward speed (both motors)
#define SPEED_TURN   140   // Inner motor speed while turning
#define SPEED_PIVOT   90   // Speed during corrective pivot

// ─── Timing ─────────────────────────────────────────────────
#define LOST_TIMEOUT  800  // ms — if both sensors off this long, stop

// ─── State machine ──────────────────────────────────────────
enum RobotState { FOLLOWING, TURNING_LEFT, TURNING_RIGHT, LOST, STOPPED };
RobotState currentState = FOLLOWING;

unsigned long lostTimer = 0;

// ─── Motor helpers ──────────────────────────────────────────

/** Run left motor forward at given speed (0–255). */
void leftForward(uint8_t spd) {
  digitalWrite(L_IN1, HIGH);
  analogWrite (L_IN2, 255 - spd);   // IN2 acts as active-low PWM
}

/** Run right motor forward at given speed (0–255). */
void rightForward(uint8_t spd) {
  analogWrite (R_IN1, spd);
  digitalWrite(R_IN2, LOW);
}

/** Brake left motor (fast stop). */
void leftStop() {
  digitalWrite(L_IN1, LOW);
  digitalWrite(L_IN2, LOW);
}

/** Brake right motor (fast stop). */
void rightStop() {
  digitalWrite(R_IN1, LOW);
  digitalWrite(R_IN2, LOW);
}

/** Move both motors forward. */
void goForward() {
  leftForward(SPEED_FWD);
  rightForward(SPEED_FWD);
}

/** Pivot left: right forward, left slow/stopped. */
void turnLeft() {
  leftForward(SPEED_PIVOT);
  rightForward(SPEED_TURN);
}

/** Pivot right: left forward, right slow/stopped. */
void turnRight() {
  leftForward(SPEED_TURN);
  rightForward(SPEED_PIVOT);
}

/** Full stop. */
void stopMotors() {
  leftStop();
  rightStop();
}

// ─── Setup ──────────────────────────────────────────────────
void setup() {
  // Sensor inputs
  pinMode(IR_LEFT,  INPUT);
  pinMode(IR_RIGHT, INPUT);

  // Motor outputs
  pinMode(L_IN1, OUTPUT);
  pinMode(L_IN2, OUTPUT);
  pinMode(R_IN1, OUTPUT);
  pinMode(R_IN2, OUTPUT);

  stopMotors();

  Serial.begin(115200);
  Serial.println(F("Line Follower Ready."));

  // Short delay before starting so you can place the robot
  delay(2000);
}

// ─── Main Loop ──────────────────────────────────────────────
void loop() {
  bool leftOnLine  = (digitalRead(IR_LEFT)  == LOW);  // LOW = black line
  bool rightOnLine = (digitalRead(IR_RIGHT) == LOW);

  // ── Decision table ──────────────────────────────────────
  //  Left  | Right | Action
  //  ──────┼───────┼──────────────────────────────────────
  //  ON    | ON    | Forward  (centred on line)
  //  ON    | OFF   | Turn left  (drifted right)
  //  OFF   | ON    | Turn right (drifted left)
  //  OFF   | OFF   | Lost — keep last correction briefly

  if (leftOnLine && rightOnLine) {
    // Perfectly centred
    currentState = FOLLOWING;
    lostTimer = 0;
    goForward();

  } else if (leftOnLine && !rightOnLine) {
    // Robot drifted right → correct left
    currentState = TURNING_LEFT;
    lostTimer = 0;
    turnLeft();

  } else if (!leftOnLine && rightOnLine) {
    // Robot drifted left → correct right
    currentState = TURNING_RIGHT;
    lostTimer = 0;
    turnRight();

  } else {
    // Both sensors off line
    if (currentState != LOST) {
      lostTimer = millis();
      currentState = LOST;
    }

    if (millis() - lostTimer < LOST_TIMEOUT) {
      // Continue last correction hoping to re-acquire line
      // (Motors keep their last PWM values — no new command)
    } else {
      // Truly lost — stop to prevent runaway
      currentState = STOPPED;
      stopMotors();
      Serial.println(F("Line lost. Stopped."));
    }
  }

  // ── Serial debug (comment out for faster loop) ──────────
  Serial.print(F("L:"));  Serial.print(leftOnLine);
  Serial.print(F(" R:")); Serial.print(rightOnLine);
  Serial.print(F(" State:"));
  switch(currentState) {
    case FOLLOWING:     Serial.println(F("FWD"));   break;
    case TURNING_LEFT:  Serial.println(F("LEFT"));  break;
    case TURNING_RIGHT: Serial.println(F("RIGHT")); break;
    case LOST:          Serial.println(F("LOST"));  break;
    case STOPPED:       Serial.println(F("STOP"));  break;
  }
}
