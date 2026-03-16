#include <PS2X_lib.h>

PS2X ps2x;

// motor trái
#define L_RPWM 5
#define L_LPWM 6

// motor phải
#define R_RPWM 9
#define R_LPWM 10

int DEADZONE = 10;
int MAX_SPEED = 200;

void setup() {

  Serial.begin(9600);

  ps2x.config_gamepad(52,51,53,50,true,true);

  pinMode(L_RPWM,OUTPUT);
  pinMode(L_LPWM,OUTPUT);
  pinMode(R_RPWM,OUTPUT);
  pinMode(R_LPWM,OUTPUT);
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
}