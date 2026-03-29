#include <PS2X_lib.h>

PS2X ps2x;

// ===== MOTOR DC (DI CHUYỂN) =====
#define L_RPWM 5
#define L_LPWM 3
#define R_RPWM 7
#define R_LPWM 8

// ===== STEPPER TB6600 (NÂNG HẠ) =====
#define STEP_PIN 11
#define DIR_PIN   9

// ===== ĐỘNG CƠ HÚT =====
#define PIN_HUT   2
#define PIN_VALVE 4

// ===== BIẾN ĐIỀU KHIỂN XE =====
int DEADZONE = 15;
int MAX_SPEED = 160;
const int TURN_MAX = 120;
float accel = 0.1;
float currentLeft = 0;
float currentRight = 0;
float a = 100.0f;

unsigned long valveTimer = 0;
bool valveActive = false;
const int valveDuration = 100;

bool ps2Connected = false;

// ================= SETUP =================
void setup() {

  Serial.begin(9600);

  int error = ps2x.config_gamepad(52,51,53,50,true,true);

  if(error == 0){
    ps2Connected = true;
    Serial.println("PS2 Connected");
  }
  else{
    Serial.println("PS2 Error");
  }

  pinMode(L_RPWM,OUTPUT);
  pinMode(L_LPWM,OUTPUT);
  pinMode(R_RPWM,OUTPUT);
  pinMode(R_LPWM,OUTPUT);

  pinMode(STEP_PIN,OUTPUT);
  pinMode(DIR_PIN,OUTPUT);

  pinMode(PIN_HUT,OUTPUT);
  digitalWrite(PIN_HUT,LOW);

  pinMode(PIN_VALVE,OUTPUT);
  digitalWrite(PIN_VALVE,LOW);

  setup_timer_1();
}

// ================= LOOP =================
void loop(){

  if(!ps2Connected) return;

  ps2x.read_gamepad();

  // ===== LÁI XE =====
  int ly = 128 - ps2x.Analog(PSS_LY);
  int rx = ps2x.Analog(PSS_RX) - 128;

  if(abs(ly)<DEADZONE) ly=0;
  if(abs(rx)<DEADZONE) rx=0;

  float speedVal = (ly/128.0)*(ly/128.0)*(ly>=0?1:-1);
  float turnVal  = (rx/128.0)*(rx/128.0)*(rx>=0?1:-1);

  currentLeft  += ((speedVal*MAX_SPEED + turnVal*TURN_MAX)-currentLeft)*accel;
  currentRight += ((speedVal*MAX_SPEED - turnVal*TURN_MAX)-currentRight)*accel;

  if(ps2x.ButtonPressed(PSB_CROSS)){a+=30;if(a>160)a=160;}
  if(ps2x.ButtonPressed(PSB_CIRCLE)){a-=30;if(a<10)a=10;}

  setMotor(L_RPWM,L_LPWM,currentLeft*(a/160.0));
  setMotor(R_RPWM,R_LPWM,currentRight*(a/160.0));

  // ===== STEPPER =====
  if(ps2x.Button(PSB_R2)){
    setStepperFrequency(2300);
    digitalWrite(DIR_PIN,LOW);
    startStepper();
  }
  else if(ps2x.Button(PSB_R1)){
    setStepperFrequency(2300);
    digitalWrite(DIR_PIN,HIGH);
    startStepper();
  }
  else{
    stopStepper();
  }

  // ===== HÚT =====
  if(ps2x.ButtonPressed(PSB_L1)){
    digitalWrite(PIN_HUT,HIGH);
    Serial.println("HUT ON");
  }

  if(ps2x.ButtonPressed(PSB_L2)){
    digitalWrite(PIN_HUT,LOW);
    Serial.println("HUT OFF");
  }

  // ===== VAN NHẢ =====
  if(ps2x.ButtonPressed(PSB_PAD_UP)){
    digitalWrite(PIN_VALVE,HIGH);
    valveTimer=millis();
    valveActive=true;
  }

  if(valveActive && millis()-valveTimer>valveDuration){
    digitalWrite(PIN_VALVE,LOW);
    valveActive=false;
  }

  delay(10);
}

// ================= HÀM HỖ TRỢ =================

void setMotor(int rpwmPin,int lpwmPin,int speed){
  speed=constrain(speed,-255,255);

  if(speed>0){
    analogWrite(rpwmPin,speed);
    analogWrite(lpwmPin,0);
  }
  else if(speed<0){
    analogWrite(rpwmPin,0);
    analogWrite(lpwmPin,-speed);
  }
  else{
    analogWrite(rpwmPin,0);
    analogWrite(lpwmPin,0);
  }
}

void setStepperFrequency(uint32_t freq){
  if(freq==0){stopStepper();return;}

  uint32_t topValue=(16000000/(8*freq))-1;

  if(topValue>65535)topValue=65535;

  ICR1=topValue;
  OCR1A=topValue/2;
}

void startStepper(){
  TCCR1A|=(1<<COM1A1);
  TCCR1B|=(1<<CS11);
}

void stopStepper(){
  TCCR1A&=~(1<<COM1A1);
  TCCR1B&=~(1<<CS11);
  digitalWrite(STEP_PIN,LOW);
}

void setup_timer_1(){
  TCCR1A=0;
  TCCR1B=0;

  TCCR1A|=(1<<WGM11);
  TCCR1B|=(1<<WGM13)|(1<<WGM12);

  setStepperFrequency(2500);
}