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
#define LIM_UP   12  // Công tắc hành trình TRÊN
#define LIM_DOWN 11  // Công tắc hành trình DƯỚI

// ===== SERVO =====
#define SERVO1_PIN 4
#define SERVO2_PIN 2

Servo servo1;
Servo servo2;

int servo1Pos = 90;
float servo2Pos = 90;
float servo2Target = 90;

#define SERVO_IN   40
#define SERVO_OUT  140
float servoSpeed = 0.2;

// ===== THAM SỐ ĐIỀU KHIỂN XE =====
int DEADZONE = 15;
int MAX_SPEED = 160;
const int TURN_MAX = 120;

float accel = 0.1;
float currentLeft = 0;
float currentRight = 0;

// ===== THÔNG SỐ ÉP XUNG STEP (CHẾ ĐỘ 1/4- - NGUỒN 24V) =====
#define STEP_DELAY_US 500   // Khoảng nghỉ 
#define STEP_PULSE_US 300   // Độ rộng xung (Cố định 20us cho TB6600)

bool ps2Connected = false;

void setup() {
  Serial.begin(9600);

  // Cấu hình PS2
  int error = ps2x.config_gamepad(52, 51, 53, 50, true, true);
  if(error == 0) ps2Connected = true;

  // Cấu hình Motor DC
  pinMode(L_RPWM, OUTPUT); pinMode(L_LPWM, OUTPUT);
  pinMode(R_RPWM, OUTPUT); pinMode(R_LPWM, OUTPUT);
  // Cấu hình TB6600
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);



setup_timer_1();
  // Khởi tạo Servo
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo1.write(servo1Pos);
  servo2.write(servo2Pos);
}

void singleStep() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(STEP_PULSE_US); 
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(STEP_DELAY_US); 
}

void loop() {
  if(!ps2Connected) return;
  ps2x.read_gamepad();

  // --- 1. LÁI XE ---
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

  // --- 2. NÂNG HẠ TAY GẮP 
  if (ps2x.Button(PSB_R1)) {

    setStepperFrequency(2000);
      digitalWrite(3,LOW);
    startStepper();
    
  } 
  else if (ps2x.Button(PSB_R2)) {
      setStepperFrequency(2000);
      digitalWrite(3,HIGH);
    startStepper();
    
  }else{
    stopStepper(); 
        }

  // --- 3. SERVO ĐIỀU KHIỂN ---
  if(ps2x.Button(PSB_PAD_UP))   servo1Pos = constrain(servo1Pos + 1, 0, 180);
  if(ps2x.Button(PSB_PAD_DOWN)) servo1Pos = constrain(servo1Pos - 1, 0, 180);
  servo1.write(servo1Pos);

  if(ps2x.ButtonPressed(PSB_L1)) servo2Target = SERVO_IN;
  if(ps2x.ButtonPressed(PSB_L2)) servo2Target = SERVO_OUT;
  servo2Pos += (servo2Target - servo2Pos) * servoSpeed;
  servo2.write(servo2Pos);
Serial.print(currentLeft);
Serial.print(      "          ");
Serial.println(currentLeft*(100.0f/160.0f));
  delay(10);
}

void setMotor(int rpwmPin, int lpwmPin, int speed) {
  speed = constrain(speed, -255, 255);
  if(speed > 0) { analogWrite(rpwmPin, speed); analogWrite(lpwmPin, 0); }
  else if(speed < 0) { analogWrite(rpwmPin, 0); analogWrite(lpwmPin, -speed); }
  else { analogWrite(rpwmPin, 0); analogWrite(lpwmPin, 0); }
}
void setStepperFrequency(uint32_t freq) {
  if (freq == 0) {
    stopStepper();
    return;
  }

  // Công thức: TOP = (F_CPU / (Prescaler * F_Target)) - 1
  // Ở đây chọn Prescaler = 8 để dải tần số rộng hơn (từ ~31Hz đến 2MHz)
  uint32_t topValue = (16000000 / (8 * freq)) - 1;
  
  // Giới hạn giá trị cho Timer 16-bit
  if (topValue > 65535) topValue = 65535;
  if (topValue < 8) topValue = 8; // Tránh tần số quá cao gây lỗi

  ICR1 = topValue;            // Cập nhật tần số mới
  OCR1A = topValue / 2;       // Duy trì Duty Cycle 50% (Xung vuông)
}

/**
 * Bật xuất xung ra chân 11
 */
void startStepper() {
  // Kết nối Timer với chân Output (Chế độ non-inverting)
  TCCR1A |= (1 << COM1A1);
  // Chạy Timer với Prescaler = 8
  TCCR1B |= (1 << CS11);
}

/**
 * Tắt xuất xung (Dừng motor)
 */
void stopStepper() {
  // Ngắt kết nối Timer với chân Output
  TCCR1A &= ~(1 << COM1A1);
  // Dừng bộ đếm Timer
  TCCR1B &= ~(1 << CS11);
  digitalWrite(STEP_PIN, LOW); // Đảm bảo chân về mức thấp
}
void setup_timer_1(){

  TCCR1A = 0;
  TCCR1B = 0;
  
  // Thiết lập Mode 14: Fast PWM với đỉnh là ICR1
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);
  
  // Bắt đầu với tần số 1kHz (hoặc tùy chọn)
  setStepperFrequency(2500); 
}