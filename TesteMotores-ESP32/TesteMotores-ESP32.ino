const int MOTOR_A_IN1 = 23;
const int MOTOR_A_IN2 = 22;
const int MOTOR_B_IN1 = 26;
const int MOTOR_B_IN2 = 27;

void testarPino(const char* motor, int pino) {
  Serial.printf("%s - GPIO %d: HIGH\n", motor, pino);
  analogWrite(pino, HIGH);
  delay(3000);
  analogWrite(pino, LOW);
  Serial.printf("%s - GPIO %d: LOW\n", motor, pino);
  delay(3000);
}

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
}

void loop() {
  testarPino("MOTOR A", MOTOR_A_IN1);
  delay(3000);
  testarPino("MOTOR A", MOTOR_A_IN2);
  delay(3000);
  testarPino("MOTOR B", MOTOR_B_IN1);
  delay(3000);
  testarPino("MOTOR B", MOTOR_B_IN2);
  delay(3000);
}