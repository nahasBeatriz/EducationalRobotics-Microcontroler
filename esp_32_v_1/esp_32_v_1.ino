#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID     = "ESP32-Robotics";
const char* AP_PASSWORD = "12345678";

// Pinos do driver de motor
#define MOTOR_A_IN1 19
#define MOTOR_A_IN2 18
#define MOTOR_B_IN3  5
#define MOTOR_B_IN4 17

// Canal PWM (para controle de velocidade via ledc)
#define PWM_CHANNEL_A1 0
#define PWM_CHANNEL_A2 1
#define PWM_CHANNEL_B3 2
#define PWM_CHANNEL_B4 3
#define PWM_FREQ       1000
#define PWM_RESOLUTION    8   // 0-255

WebServer server(80);

int currentSpeed = 200; // velocidade padrão (0-255)

// ---------- helpers de motor ----------

void motorStop() {
  ledcWrite(PWM_CHANNEL_A1, 0);
  ledcWrite(PWM_CHANNEL_A2, 0);
  ledcWrite(PWM_CHANNEL_B3, 0);
  ledcWrite(PWM_CHANNEL_B4, 0);
}

void motorFrente() {
  ledcWrite(PWM_CHANNEL_A1, currentSpeed);
  ledcWrite(PWM_CHANNEL_A2, 0);
  ledcWrite(PWM_CHANNEL_B3, currentSpeed);
  ledcWrite(PWM_CHANNEL_B4, 0);
}

void motorTras() {
  ledcWrite(PWM_CHANNEL_A1, 0);
  ledcWrite(PWM_CHANNEL_A2, currentSpeed);
  ledcWrite(PWM_CHANNEL_B3, 0);
  ledcWrite(PWM_CHANNEL_B4, currentSpeed);
}

void motorEsquerda() {
  ledcWrite(PWM_CHANNEL_A1, 0);
  ledcWrite(PWM_CHANNEL_A2, currentSpeed);
  ledcWrite(PWM_CHANNEL_B3, currentSpeed);
  ledcWrite(PWM_CHANNEL_B4, 0);
}

void motorDireita() {
  ledcWrite(PWM_CHANNEL_A1, currentSpeed);
  ledcWrite(PWM_CHANNEL_A2, 0);
  ledcWrite(PWM_CHANNEL_B3, 0);
  ledcWrite(PWM_CHANNEL_B4, currentSpeed);
}

// ---------- HTTP ----------

void setCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void sendJSON(int code, String body) {
  setCORSHeaders();
  server.send(code, "application/json", body);
}

void handleStatus() {
  sendJSON(200, "{\"speed\":" + String(currentSpeed) + "}");
}

void handleCommand() {
  if (!server.hasArg("action")) {
    sendJSON(400, "{\"error\":\"parametro action ausente\"}");
    return;
  }

  if (server.hasArg("speed")) {
    int s = server.arg("speed").toInt();
    if (s >= 0 && s <= 255) currentSpeed = s;
  }

  String action = server.arg("action");

  if (action == "frente") {
    motorFrente();
    sendJSON(200, "{\"result\":\"frente\"}");
  } else if (action == "tras") {
    motorTras();
    sendJSON(200, "{\"result\":\"tras\"}");
  } else if (action == "esquerda") {
    motorEsquerda();
    sendJSON(200, "{\"result\":\"esquerda\"}");
  } else if (action == "direita") {
    motorDireita();
    sendJSON(200, "{\"result\":\"direita\"}");
  } else if (action == "parar") {
    motorStop();
    sendJSON(200, "{\"result\":\"parado\"}");
  } else {
    sendJSON(400, "{\"error\":\"action desconhecida: " + action + "\"}");
  }

  Serial.println("[CMD] action=" + action + " speed=" + String(currentSpeed));
}

void handleNotFound() {
  sendJSON(404, "{\"error\":\"rota nao encontrada\"}");
}

// ---------- setup / loop ----------

void setup() {
  Serial.begin(115200);
  delay(1000);

  ledcSetup(PWM_CHANNEL_A1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_A2, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B3, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B4, PWM_FREQ, PWM_RESOLUTION);

  ledcAttachPin(MOTOR_A_IN1, PWM_CHANNEL_A1);
  ledcAttachPin(MOTOR_A_IN2, PWM_CHANNEL_A2);
  ledcAttachPin(MOTOR_B_IN3, PWM_CHANNEL_B3);
  ledcAttachPin(MOTOR_B_IN4, PWM_CHANNEL_B4);

  motorStop();

  WiFi.disconnect(true);
  delay(500);
  WiFi.mode(WIFI_AP);
  delay(500);

  bool ok = WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(500);

  Serial.println("\n=== ESP32 AP ===");
  if (ok) {
    Serial.println("AP criado com sucesso!");
  } else {
    Serial.println("FALHOU ao criar AP — reiniciando...");
    delay(3000);
    ESP.restart();
  }
  Serial.println("SSID : " + String(AP_SSID));
  Serial.println("IP   : " + WiFi.softAPIP().toString());
  Serial.println("================\n");

  server.on("/status", HTTP_GET, handleStatus);
  server.on("/cmd",    HTTP_GET, handleCommand);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Servidor HTTP rodando na porta 80");
}

void loop() {
  server.handleClient();
}