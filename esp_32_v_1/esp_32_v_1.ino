#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID     = "ESP32-Robotics";
const char* AP_PASSWORD = "12345678";

#define IN1 0
#define IN2 1
#define IN3 3
#define IN4 4

#define PWM_FREQ       1000
#define PWM_RESOLUTION    8

WebServer server(80);
int currentSpeed = 200;

// ---------- motor ----------

void setMotors(int a1, int a2, int b1, int b2) {
  ledcWrite(0, a1); ledcWrite(1, a2);
  ledcWrite(2, b1); ledcWrite(3, b2);
}

void motorStop()     { setMotors(0, 0, 0, 0); }
void motorFrente()   { setMotors(currentSpeed, 0, currentSpeed, 0); }
void motorTras()     { setMotors(0, currentSpeed, 0, currentSpeed); }
void motorEsquerda() { setMotors(0, currentSpeed, currentSpeed, 0); }
void motorDireita()  { setMotors(currentSpeed, 0, 0, currentSpeed); }

// ---------- HTTP ----------

void sendJSON(int code, String body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
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

  if      (action == "frente")   motorFrente();
  else if (action == "tras")     motorTras();
  else if (action == "esquerda") motorEsquerda();
  else if (action == "direita")  motorDireita();
  else if (action == "parar")    motorStop();
  else {
    sendJSON(400, "{\"error\":\"action desconhecida: " + action + "\"}");
    return;
  }

  Serial.println("[CMD] " + action + " spd=" + String(currentSpeed));
  sendJSON(200, "{\"result\":\"" + action + "\"}");
}

// ---------- setup / loop ----------

void setup() {
  Serial.begin(115200);
  delay(1000);

  int pins[] = {IN1, IN2, IN3, IN4};
  for (int i = 0; i < 4; i++) {
    ledcSetup(i, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(pins[i], i);
  }
  motorStop();

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(500);

  Serial.println("\n=== ESP32 AP ===");
  if (!ok) { Serial.println("FALHOU — reiniciando..."); delay(3000); ESP.restart(); }
  Serial.println("SSID: " + String(AP_SSID));
  Serial.println("IP  : " + WiFi.softAPIP().toString());

  server.on("/status", HTTP_GET, handleStatus);
  server.on("/cmd",    HTTP_GET, handleCommand);
  server.onNotFound([]() { sendJSON(404, "{\"error\":\"rota nao encontrada\"}"); });
  server.begin();
  Serial.println("HTTP na porta 80\n");
}

void loop() { server.handleClient(); }