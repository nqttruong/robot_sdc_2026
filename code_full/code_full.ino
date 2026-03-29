#include <PS2X_lib.h>
#include <ServoTimer2.h> // Thay đổi thư viện ở đây

PS2X ps2x;

// ===== MOTOR DC =====
#define L_RPWM 5
#define L_LPWM 3
#define R_RPWM 7
#define R_LPWM 8

// ===== STEPPER TB6600 (Sử dụng Timer 1) =====
#define STEP_PIN 11
#define DIR_PIN   9


// ===== SERVO (Sử dụng Timer 2 qua ServoTimer2) =====
#define SERVO1_PIN 4 
#define SERVO2_PIN 2

ServoTimer2 servo1; // Đổi kiểu dữ liệu
ServoTimer2 servo2;

int servoPos = 90; // Vẫn giữ đơn vị là độ (0-180) để dễ tính toán

#define SERVO_MIN 40
#define SERVO_MAX 140

// ===== XE =====
int DEADZONE = 15;
int MAX_SPEED = 160;
const int TURN_MAX = 120;
float accel = 0.1;
float currentLeft = 0;
float currentRight = 0;
float a = 100.0f;

bool ps2Connected = false;

// Hàm phụ để chuyển từ độ sang Microseconds cho ServoTimer2
int angleToUs(int angle) {
  return map(angle, 0, 180, 750, 2250);
}

// ================= SETUP =================
void setup() {
  Serial.begin(9600);

  int error = ps2x.config_gamepad(52, 51, 53, 50, true, true);

  if(error == 0){
    ps2Connected = true;
    Serial.println("PS2 Controller Connected!");
  }
  else if(error == 1){
    Serial.println("PS2 Controller NOT found!");
  }
  else if(error == 2){
    Serial.println("PS2 Controller found but NOT accepting commands");
  }
  else if(error == 3){
    Serial.println("PS2 Controller refusing to enter Pressures mode");
  }

  pinMode(L_RPWM, OUTPUT);
  pinMode(L_LPWM, OUTPUT);
  pinMode(R_RPWM, OUTPUT);
  pinMode(R_LPWM, OUTPUT);
pinMode(12,0);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  setup_timer_1();

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  servo1.write(angleToUs(servoPos));
  servo2.write(angleToUs(180 - servoPos));
}

// ================= LOOP =================
void loop() {
  if(!ps2Connected) return;
  ps2x.read_gamepad();

  // ===== 1. LÁI XE =====
  int ly = 128 - ps2x.Analog(PSS_LY);
  int rx = ps2x.Analog(PSS_RX) - 128;

  Serial.print("LY: ");
Serial.print(ps2x.Analog(PSS_LY));

Serial.print("  RX: ");
Serial.print(ps2x.Analog(PSS_RX));

  if(abs(ly) < DEADZONE) ly = 0;
  if(abs(rx) < DEADZONE) rx = 0;

  float speedVal = (ly / 128.0) * (ly / 128.0) * (ly >= 0 ? 1 : -1);
  float turnVal  = (rx / 128.0) * (rx / 128.0) * (rx >= 0 ? 1 : -1);

  currentLeft  += ((speedVal * MAX_SPEED + turnVal * TURN_MAX) - currentLeft) * accel;
  currentRight += ((speedVal * MAX_SPEED - turnVal * TURN_MAX) - currentRight) * accel;

  if (ps2x.ButtonPressed(PSB_CROSS)) { a += 30; if (a > 160) a = 160; }
  if (ps2x.ButtonPressed(PSB_CIRCLE)) { a -= 30; if (a < 10) a = 10; }

  setMotor(L_RPWM, L_LPWM, currentLeft*(a/160.0f));
  setMotor(R_RPWM, R_LPWM, currentRight*(a/160.0f));

  // ===== 2. NÂNG HẠ TAY GẮP (Băm xung Timer 1) =====
  if (ps2x.Button(PSB_R2)) {
    Serial.print("  R2: ");
Serial.println(ps2x.Button(PSB_R2));
    setStepperFrequency(2300);
    digitalWrite(DIR_PIN, LOW);
    startStepper();
  }
  else if (ps2x.Button(PSB_R1)) {
    Serial.print("  R1: ");
Serial.print(ps2x.Button(PSB_R1));
    setStepperFrequency(2300);
    digitalWrite(DIR_PIN, HIGH);
    startStepper();
  }
  else {
    stopStepper();
  }

  // ===== 3. SERVO KẸP (Điều khiển qua Timer 2) =====
  if(ps2x.Button(PSB_L1)){
    servoPos += 20;
    Serial.print("Nhan L1 roi      ");
      Serial.print("  L1: ");
Serial.print(ps2x.Button(PSB_L1));
  }




  if(ps2x.Button(PSB_L2)){
    servoPos -= 20;
    Serial.print("Nhan L2 roi       ");
      Serial.print("  L2: ");
Serial.print(ps2x.Button(PSB_L2));
  }


  if(ps2x.ButtonPressed(PSB_TRIANGLE)){
    servoPos = 90;
  }

  servoPos = constrain(servoPos, SERVO_MIN, SERVO_MAX);

  // Ghi giá trị đã được map sang Microseconds
  servo1.write(angleToUs(servoPos));
  servo2.write(angleToUs(180 - servoPos));

  // Debug
  Serial.print("Servo Degree: ");
  Serial.print(servoPos);
  Serial.print(" | PWM Us: ");
  Serial.println(angleToUs(servoPos));

  delay(15); // Tăng delay một chút để PS2 và Servo ổn định hơn
}


void setMotor(int rpwmPin, int lpwmPin, int speed) {
  speed = constrain(speed, -255, 255);
  if(speed > 0) { analogWrite(rpwmPin, speed); analogWrite(lpwmPin, 0); }
  else if(speed < 0) { analogWrite(rpwmPin, 0); analogWrite(lpwmPin, -speed); }
  else { analogWrite(rpwmPin, 0); analogWrite(lpwmPin, 0); }
}

void setStepperFrequency(uint32_t freq) {
  if (freq == 0) { stopStepper(); return; }
  uint32_t topValue = (16000000 / (8 * freq)) - 1;
  if (topValue > 65535) topValue = 65535;
  if (topValue < 8) topValue = 8;
  ICR1 = topValue;
  OCR1A = topValue / 2;
}

void startStepper() { TCCR1A |= (1 << COM1A1); TCCR1B |= (1 << CS11); }
void stopStepper() { TCCR1A &= ~(1 << COM1A1); TCCR1B &= ~(1 << CS11); digitalWrite(STEP_PIN, LOW); }

void setup_timer_1(){
  TCCR1A = 0; TCCR1B = 0;
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);
  setStepperFrequency(2500);
}