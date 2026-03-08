#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* ================= WIFI ================= */

const char* ssid = "<hotspot-name>";
const char* password = "<hotspot- password>";

/* ================= THINGSPEAK ================= */

String apiKey = "<THINGSPEAK API KEY>";
const char* server = "http://api.thingspeak.com/update";

/* ================= PIN DEFINITIONS ================= */

#define PH_PIN 35
#define ONE_WIRE_BUS 4

/* ================= TEMPERATURE SENSOR ================= */

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/* ================= FILTER SETTINGS ================= */

int samples = 10;

/* ================= SETUP ================= */

void setup() {

  Serial.begin(115200);
  sensors.begin();

  connectWiFi();

  Serial.println("Water Monitoring System Started...");
}

/* ================= MAIN LOOP ================= */

void loop() {

  float phValue = readPH();
  float temperatureValue = readTemperature();

  Serial.println("\n----- SENSOR DATA -----");

  Serial.print("pH: ");
  Serial.println(phValue);

  Serial.print("Temperature (°C): ");
  Serial.println(temperatureValue);

  detectAnomalies(phValue, temperatureValue);

  uploadToThingSpeak(phValue, temperatureValue);

  delay(15000);   // ThingSpeak minimum interval
}

/* ================= WIFI CONNECTION ================= */

void connectWiFi() {

  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
}

/* ================= pH SENSOR ================= */

float readPH() {

  float total = 0;

  for (int i = 0; i < samples; i++) {
    total += analogRead(PH_PIN);
    delay(10);
  }

  float raw = total / samples;
  float voltage = raw * (3.3 / 4095.0);
  float ph = 7 + ((2.5 - voltage) / 0.18);

  return ph;
}

/* ================= TEMPERATURE SENSOR ================= */

float readTemperature() {

  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  if (temp == DEVICE_DISCONNECTED_C) {
    Serial.println("Temperature sensor not detected!");
    return -127;
  }

  return temp;
}

/* ================= ANALYTICS ================= */

void detectAnomalies(float ph, float temp) {

  if (ph < 6.5 || ph > 8.5)
    Serial.println("⚠ ALERT: pH OUT OF SAFE RANGE");

  if (temp > 35)
    Serial.println("⚠ ALERT: HIGH TEMPERATURE");
}

/* ================= THINGSPEAK UPLOAD ================= */

void uploadToThingSpeak(float ph, float temp) {

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  HTTPClient http;

  String url = server;
  url += "?api_key=" + apiKey;
  url += "&field1=" + String(ph);
  url += "&field2=" + String(temp);

  http.begin(url);
  int httpCode = http.GET();

  Serial.print("ThingSpeak Response: ");
  Serial.println(httpCode);

  http.end();
}