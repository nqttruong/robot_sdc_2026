#include <PS2X_lib.h>

PS2X ps2x;

// motor trái
#define L_RPWM 5
#define L_LPWM 6

// motor phải
#define R_RPWM 9
#define R_LPWM 10

// TB6600
#define STEP_PIN 2
#define DIR_PIN  3

int DEADZONE = 12;
int MAX_SPEED = 200;

unsigned long lastStepTime = 0;
int stepDelay = 6000;   // nhỏ = quay nhanh
bool stepState = LOW;

void setup() {

  Serial.begin(9600);

  ps2x.config_gamepad(52,51,53,50,true,true);

  pinMode(L_RPWM,OUTPUT);
  pinMode(L_LPWM,OUTPUT);
  pinMode(R_RPWM,OUTPUT);
  pinMode(R_LPWM,OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
}

void setMotor(int rpwmPin,int lpwmPin,int speed)
{
  speed = constrain(speed,-255,255);

  if(speed > 0)
  {
    analogWrite(rpwmPin,speed);
    analogWrite(lpwmPin,0);
  }
  else if(speed < 0)
  {
    analogWrite(rpwmPin,0);
    analogWrite(lpwmPin,-speed);
  }
  else
  {
    analogWrite(rpwmPin,0);
    analogWrite(lpwmPin,0);
  }
}

void loop() {

  ps2x.read_gamepad();

  // ===== JOYSTICK =====
  int ly = 128 - ps2x.Analog(PSS_LY);   // tiến/lùi
  int rx = ps2x.Analog(PSS_RX) - 128;   // rẽ

  if(abs(ly) < DEADZONE) ly = 0;
  if(abs(rx) < DEADZONE) rx = 0;

  float speed = ly / 128.0;
  float turn  = rx / 128.0;

  speed = speed * abs(speed);
  turn  = turn  * abs(turn);

  int motorSpeed = speed * MAX_SPEED;
  int motorTurn  = turn  * MAX_SPEED;

  // ===== NÚT QUAY =====
  bool btnLeft  = ps2x.Button(PSB_L1);
  bool btnRight = ps2x.Button(PSB_L2);

  int leftMotor, rightMotor;

  if(btnLeft)
  {
    leftMotor  = -MAX_SPEED;
    rightMotor =  MAX_SPEED;
  }
  else if(btnRight)
  {
    leftMotor  =  MAX_SPEED;
    rightMotor = -MAX_SPEED;
  }
  else
  {
    leftMotor  = motorSpeed + motorTurn;
    rightMotor = motorSpeed - motorTurn;

    if(motorSpeed == 0)
    {
      leftMotor  = motorTurn;
      rightMotor = -motorTurn;
    }
  }

  leftMotor  = constrain(leftMotor, -255, 255);
  rightMotor = constrain(rightMotor, -255, 255);

  setMotor(L_RPWM, L_LPWM, leftMotor);
  setMotor(R_RPWM, R_LPWM, rightMotor);

  // ===== STEP MOTOR TB6600 =====
  bool r1 = ps2x.Button(PSB_R1);
  bool r2 = ps2x.Button(PSB_R2);

  if(r1 || r2)
  {
    digitalWrite(DIR_PIN, r1); // R1 thuận, R2 ngược

    // 👉 chỉnh tốc độ step bằng joystick phải (trục Y)
    int ry = 128 - ps2x.Analog(PSS_RY);
    if(abs(ry) < DEADZONE) ry = 0;

    // map thành delay (nhanh/chậm)
    stepDelay = map(abs(ry), 0, 128, 1500, 200);

    if(micros() - lastStepTime >= stepDelay)
    {
      lastStepTime = micros();

      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(5);   // xung chuẩn TB6600
      digitalWrite(STEP_PIN, LOW);
    }
  }
  else
  {
    digitalWrite(STEP_PIN, LOW);
  }
}