#include <PS2X_lib.h>

PS2X ps2x;

// motor trái
#define L_RPWM 5
#define L_LPWM 6

// motor phải
#define R_RPWM 9
#define R_LPWM 10

#define STEP_PIN 2
#define DIR_PIN  3

int DEADZONE = 10;
int MAX_SPEED = 200;

unsigned long lastStepTime = 0;
int stepDelay = 800;   // tốc độ step
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

void stepMotor(bool dir, int steps, int speedDelay)
{
  digitalWrite(DIR_PIN, dir);

  for(int i = 0; i < steps; i++)
  {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(speedDelay);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(speedDelay);
  }
}

void loop() {

  ps2x.read_gamepad();

  int lx = ps2x.Analog(PSS_LX) - 128;
  int ly = 128 - ps2x.Analog(PSS_LY);

  if(abs(lx) < DEADZONE) lx = 0;
  if(abs(ly) < DEADZONE) ly = 0;

  int turn  = map(lx,-128,127,-MAX_SPEED,MAX_SPEED);
  int speed = map(ly,-128,127,-MAX_SPEED,MAX_SPEED);

  int leftMotor  = speed + turn;
  int rightMotor = speed - turn;

  leftMotor  = constrain(leftMotor,-255,255);
  rightMotor = constrain(rightMotor,-255,255);  

  setMotor(L_RPWM,L_LPWM,leftMotor);
  setMotor(R_RPWM,R_LPWM,rightMotor);
  Serial.println(ly);
  delay(20);
    // đọc nút
  bool r1 = ps2x.Button(PSB_R1);
  bool r2 = ps2x.Button(PSB_R2);
  if(r1 || r2)
  {
    digitalWrite(DIR_PIN, r1); // R1 = thuận, R2 = ngược

    if(micros() - lastStepTime >= stepDelay)
    {
      lastStepTime = micros();

      stepState = !stepState;
      digitalWrite(STEP_PIN, stepState);
    }
  }
  else
  {
    digitalWrite(STEP_PIN, LOW);
  }
}