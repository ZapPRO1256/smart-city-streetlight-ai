/*
  Smart Street Light – AI patterns + LLM policy
  Компоненти:
  - ESP32
  - LDR (фоторезистор)
  - PIR сенсор
  - RGB LED (через PWM)
  - Wi-Fi + WebServer
*/

#include <WiFi.h>
#include <WebServer.h>

// ---- WiFi ----
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ---- Сенсори ----
#define LDR_PIN 35
#define PIR_PIN 19
#define RED_PIN 16
#define GREEN_PIN 17
#define BLUE_PIN 18

// ---- Змінні стану ----
int ldrValue = 0;
int pirState = 0;
bool aiMode = true;  // режим AI за замовчуванням
int brightness = 0;

// ---- WebServer ----
WebServer server(80);

// ---- Функція для встановлення яскравості ----
void setBrightness(int level) {
  level = constrain(level, 0, 255);
  analogWrite(RED_PIN, level);
  analogWrite(GREEN_PIN, level);
  analogWrite(BLUE_PIN, level);
}

// ---- AI логіка ----
int computeAIbrightness(int light, int motion) {
  // Простий "AI-патерн": аналіз освітлення + руху
  if (light > 2500) return 0;              // день
  if (motion == 1 && light < 800) return 255; // темно і є рух
  if (motion == 1 && light < 1500) return 180;
  if (motion == 0 && light < 1000) return 80; // темно, але тихо
  return 20;                                // ніч, без руху
}

// ---- Веб інтерфейс ----
void handleRoot() {
  String html = "<h2>🌃 Smart Street Light</h2>";
  html += "<p>LDR value: " + String(ldrValue) + "</p>";
  html += "<p>PIR state: " + String(pirState ? "Motion detected" : "No motion") + "</p>";
  html += "<p>Brightness: " + String(brightness) + "</p>";
  html += "<p>Mode: " + String(aiMode ? "AI" : "Manual") + "</p>";
  html += "<a href=\"/toggle\">🧠 Toggle Mode</a><br><br>";
  html += "<a href=\"/policy\">📜 Get LLM Policy</a><br><br>";
  html += "<a href=\"/bright_up\">🔆 Bright+</a> | <a href=\"/bright_down\">🔅 Bright-</a>";
  server.send(200, "text/html", html);
}

// ---- Перемикання режиму ----
void handleToggle() {
  aiMode = !aiMode;
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---- Ручне керування ----
void handleBrightUp() {
  brightness = min(255, brightness + 25);
  setBrightness(brightness);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleBrightDown() {
  brightness = max(0, brightness - 25);
  setBrightness(brightness);
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---- LLM “policy” (імітація відповіді) ----
void handlePolicy() {
  String policy = "LLM Policy Suggestion:<br>";
  policy += "- Lower base brightness by 20% after 23:00.<br>";
  policy += "- Increase AI sensitivity if motion occurs frequently.<br>";
  policy += "- Adjust cooldown time to 60s when no motion detected.";
  server.send(200, "text/html", policy);
}

// ---- SETUP ----
void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Wi-Fi підключення
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Веб-маршрути
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/bright_up", handleBrightUp);
  server.on("/bright_down", handleBrightDown);
  server.on("/policy", handlePolicy);
  server.begin();
  Serial.println("WebServer started!");
}

// ---- LOOP ----
void loop() {
  server.handleClient();

  ldrValue = analogRead(LDR_PIN);
  pirState = digitalRead(PIR_PIN);

  if (aiMode) {
    brightness = computeAIbrightness(ldrValue, pirState);
    setBrightness(brightness);
  }

  delay(200);
}
