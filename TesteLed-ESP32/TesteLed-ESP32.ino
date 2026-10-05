#include <WiFi.h>
#include <WebServer.h>

const char* AP_SSID     = "ESP32-Robotics";
const char* AP_PASSWORD = "12345678";

const int LED_PIN = 5;

WebServer server(80);

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
  String state = digitalRead(LED_PIN) == HIGH ? "on" : "off";
  sendJSON(200, "{\"led\":\"" + state + "\"}");
}

void handleCommand() {
  if (!server.hasArg("action")) {
    sendJSON(400, "{\"error\":\"parametro action ausente\"}");
    return;
  }

  String action = server.arg("action");

  if (action == "on") {
    digitalWrite(LED_PIN, HIGH);
    sendJSON(200, "{\"result\":\"LED ligado\"}");
    Serial.println("[CMD] LED -> ON");

  } else if (action == "off") {
    digitalWrite(LED_PIN, LOW);
    sendJSON(200, "{\"result\":\"LED desligado\"}");
    Serial.println("[CMD] LED -> OFF");

  } else if (action == "toggle") {
    int newState = !digitalRead(LED_PIN);
    digitalWrite(LED_PIN, newState);
    String state = newState == HIGH ? "on" : "off";
    sendJSON(200, "{\"result\":\"LED " + state + "\"}");
    Serial.println("[CMD] LED -> TOGGLE (" + state + ")");

  } else {
    sendJSON(400, "{\"error\":\"action desconhecida: " + action + "\"}");
  }
}

void handleNotFound() {
  sendJSON(404, "{\"error\":\"rota nao encontrada\"}");
}

void setup() {
  Serial.begin(115200);
  delay(1000); // aguarda Serial estabilizar

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ── força modo AP antes de subir ──────────────────────────
  WiFi.disconnect(true);
  delay(500);
  WiFi.mode(WIFI_AP);
  delay(500);

  bool ok = WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(500); // aguarda AP estabilizar

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