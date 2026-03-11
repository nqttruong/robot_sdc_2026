#include <PS2X_lib.h>
#include <Servo.h>

// ===== PS2 =====
PS2X ps2x;

// ===== Servo =====
Servo servoL;
Servo servoR;

// ===== Chân =====
#define SERVO_L_PIN 30
#define SERVO_R_PIN 31

// ===== Tốc độ (CHỈNH THEO SERVO THỰC TẾ) =====
int STOP = 90;   // điểm dừng
int FAST = 120;  // tiến nhanh
int BACK = 60;   // lùi

void setup() {
  Serial.begin(9600);

  // PS2: (CLK, CMD, CS, DAT)
  int error = ps2x.config_gamepad(52, 51, 53, 50, true, true);
  if (error == 0) Serial.println("PS2 OK");
  else Serial.println("PS2 FAIL");

  servoL.attach(SERVO_L_PIN);
  servoR.attach(SERVO_R_PIN);

  // Dừng an toàn khi khởi động
  servoL.write(STOP);
  servoR.write(STOP);
  delay(500);
}

void loop() {
  ps2x.read_gamepad();

  if (ps2x.Button(PSB_PAD_UP)) {
    // TIẾN
    servoL.write(FAST);
    servoR.write(FAST);
  }
  else if (ps2x.Button(PSB_PAD_DOWN)) {
    // LÙI
    servoL.write(BACK);
    servoR.write(BACK);
  }
  else if (ps2x.Button(PSB_PAD_LEFT)) {
    // QUAY TRÁI
    servoL.write(STOP);
    servoR.write(FAST);
  }
  else if (ps2x.Button(PSB_PAD_RIGHT)) {
    // QUAY PHẢI
    servoL.write(FAST);
    servoR.write(STOP);
  }
  else {
    // DỪNG
    servoL.write(STOP);
    servoR.write(STOP);
  }

  delay(20);
}