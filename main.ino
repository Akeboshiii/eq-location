#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>  // Required for HTTPS

const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASS";
const char* serverUrl = "https://your-api.com/endpoint";  // HTTPS URL

SoftwareSerial gpsSerial(4, 3);   // RX=4 (GPS TX), TX=3 (GPS RX)
TinyGPSPlus gps;

double lastLat = 0.0;
double lastLon = 0.0;

String finalLat = "0.000000";
String finalLon = "0.000000";
bool hasFixEver = false; 

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
    if (gps.location.isValid()) {
      lastLat = gps.location.lat();
      lastLon = gps.location.lng();
      hasFixEver = true;
    }
  }

  updateCoordinates();

  if (WiFi.status() == WL_CONNECTED) {
    BearSSL::WiFiClientSecure client;
    client.setInsecure();  // ✅ ALLOW SELF-SIGNED / NO CERT CHECK

    HTTPClient http;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"lat\": " + finalLat + ", \"lon\": " + finalLon + "}";

    int httpCode = http.PUT(jsonPayload);

    Serial.print("Response code: ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      Serial.println("Response body: " + http.getString());
    }

    http.end();
  }

  delay(5000);
}

void updateCoordinates() {
  if (gps.location.isValid()) {
    finalLat = String(gps.location.lat(), 6);
    finalLon = String(gps.location.lng(), 6);
  } else if (hasFixEver) {
    finalLat = String(lastLat, 6);
    finalLon = String(lastLon, 6);
  } else {
    finalLat = "0.000000";
    finalLon = "0.000000";
  }

  Serial.println("GPS -> " + finalLat + ", " + finalLon);
}
