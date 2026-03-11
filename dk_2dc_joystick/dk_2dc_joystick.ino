#include <PS2X_lib.h>
#include <Servo.h>

PS2X ps2x;
Servo servoL;
Servo servoR;

#define SERVO_L_PIN 30
#define SERVO_R_PIN 31

// ===== CHỈNH THEO SERVO THỰC TẾ =====
int STOP = 90;        // điểm dừng
int MAX_SPEED = 30;   // biên độ tốc độ (+/-)

// deadzone joystick
int DEADZONE = 10;

void setup() {
  Serial.begin(9600);

  // PS2: CLK, CMD, CS, DAT
  ps2x.config_gamepad(52, 51, 53, 50, true, true);

  servoL.attach(SERVO_L_PIN);
  servoR.attach(SERVO_R_PIN);

  servoL.write(STOP);
  servoR.write(STOP);
  delay(500);
}

void loop() {
  ps2x.read_gamepad();

  // Đọc joystick trái
  int lx = ps2x.Analog(PSS_LX) - 128; // trái (-) → phải (+)
  int ly = 128 - ps2x.Analog(PSS_LY); // lên (+) → xuống (-)

  // Deadzone
  if (abs(lx) < DEADZONE) lx = 0;
  if (abs(ly) < DEADZONE) ly = 0;

  // Map joystick → tốc độ
  int turn  = map(lx, -128, 127, -MAX_SPEED, MAX_SPEED);
  int speed = map(ly, -128, 127, -MAX_SPEED, MAX_SPEED);

  int leftSpeed  = STOP + speed + turn;
  int rightSpeed = STOP - speed + turn;

  // Giới hạn an toàn
  leftSpeed  = constrain(leftSpeed,  STOP - MAX_SPEED, STOP + MAX_SPEED);
  rightSpeed = constrain(rightSpeed, STOP - MAX_SPEED, STOP + MAX_SPEED);

  servoL.write(leftSpeed);
  servoR.write(rightSpeed);

  delay(20);
}