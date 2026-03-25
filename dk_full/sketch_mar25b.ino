#include <PS2X_lib.h>
#include <Servo.h>

PS2X ps2x;

// ===== MOTOR DC (Di chuyển xe) =====
#define L_RPWM 5
#define L_LPWM 6
#define R_RPWM 9
#define R_LPWM 10

// ===== TB6600 (Nâng hạ tay gắp) =====
#define STEP_PIN 11
#define DIR_PIN  3
#define LIM_UP   12

// ===== SERVO =====
#define SERVO1_PIN 7
#define SERVO2_PIN 8

Servo servo1;
Servo servo2;

int servo1Pos = 90;
int servo2Pos = 90;

#define SERVO_MIN 45
#define SERVO_MAX 135

// ===== THAM SỐ ĐIỀU KHIỂN XE =====
int DEADZONE = 15;
int MAX_SPEED = 160;
const int TURN_MAX = 120;

float accel = 0.1;
float currentLeft = 0;
float currentRight = 0;

// ===== STEP =====
#define STEP_DELAY_US 500
#define STEP_PULSE_US 300

bool ps2Connected = false;

void setup() {

  Serial.begin(9600);

  int error = ps2x.config_gamepad(52, 51, 53, 50, true, true);
  if(error == 0) ps2Connected = true;

  pinMode(L_RPWM, OUTPUT);
  pinMode(L_LPWM, OUTPUT);
  pinMode(R_RPWM, OUTPUT);
  pinMode(R_LPWM, OUTPUT);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  setup_timer_1();

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  servo1.write(servo1Pos);
  servo2.write(servo2Pos);
}

void loop() {

  if(!ps2Connected) return;

  ps2x.read_gamepad();

  // ===== 1. LÁI XE =====
  int ly = 128 - ps2x.Analog(PSS_LY);
  int rx = ps2x.Analog(PSS_RX) - 128;

  if(abs(ly) < DEADZONE) ly = 0;
  if(abs(rx) < DEADZONE) rx = 0;

  float speedVal = (ly / 128.0) * (ly / 128.0) * (ly >= 0 ? 1 : -1);
  float turnVal  = (rx / 128.0) * (rx / 128.0) * (rx >= 0 ? 1 : -1);

  currentLeft  += ((speedVal * MAX_SPEED + turnVal * TURN_MAX) - currentLeft) * accel;
  currentRight += ((speedVal * MAX_SPEED - turnVal * TURN_MAX) - currentRight) * accel;

  setMotor(L_RPWM, L_LPWM, currentLeft*(100.0f/160.0f));
  setMotor(R_RPWM, R_LPWM, currentRight*(100.0f/160.0f));

  // ===== 2. NÂNG HẠ TAY GẮP =====
  if (ps2x.Button(PSB_R1)) {

    setStepperFrequency(2000);
    digitalWrite(DIR_PIN,LOW);
    startStepper();

  }
  else if (ps2x.Button(PSB_R2)) {

    setStepperFrequency(2000);
    digitalWrite(DIR_PIN,HIGH);
    startStepper();

  }
  else {

    stopStepper();

  }

  // ===== 3. SERVO KẸP =====

  if(ps2x.Button(PSB_L1)) {

      servo1Pos += 8;
      servo2Pos -= 8;

  }

  if(ps2x.Button(PSB_L2)) {

      servo1Pos -= 8;
      servo2Pos += 8;

  }

  servo1Pos = constrain(servo1Pos, SERVO_MIN, SERVO_MAX);
  servo2Pos = constrain(servo2Pos, SERVO_MIN, SERVO_MAX);

  servo1.write(servo1Pos);
  servo2.write(servo2Pos);

  delay(10);
}

void setMotor(int rpwmPin, int lpwmPin, int speed) {

  speed = constrain(speed, -255, 255);

  if(speed > 0) {

    analogWrite(rpwmPin, speed);
    analogWrite(lpwmPin, 0);

  }

  else if(speed < 0) {

    analogWrite(rpwmPin, 0);
    analogWrite(lpwmPin, -speed);

  }

  else {

    analogWrite(rpwmPin, 0);
    analogWrite(lpwmPin, 0);

  }
}

void setStepperFrequency(uint32_t freq) {

  if (freq == 0) {
    stopStepper();
    return;
  }

  uint32_t topValue = (16000000 / (8 * freq)) - 1;

  if (topValue > 65535) topValue = 65535;
  if (topValue < 8) topValue = 8;

  ICR1 = topValue;
  OCR1A = topValue / 2;
}

void startStepper() {

  TCCR1A |= (1 << COM1A1);
  TCCR1B |= (1 << CS11);

}

void stopStepper() {

  TCCR1A &= ~(1 << COM1A1);
  TCCR1B &= ~(1 << CS11);

  digitalWrite(STEP_PIN, LOW);

}

void setup_timer_1(){

  TCCR1A = 0;
  TCCR1B = 0;

  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);

  setStepperFrequency(2500);

}