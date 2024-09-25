// Set board to ESP32 Wrover Module
// Set Core Debug Level to Debug to see the WiFi channel scan

#include <WiFi.h>
#include <WiFiMulti.h>

#define WIFI_SSID "<ADD_YOUR_WIFI_SSID_HERE>"
#define WIFI_PASSWORD "<ADD_YOUR_WIFI_PASSWORD_HERE>"

void setup() {
  delay(1000);
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  delay(1000);

  WiFiMulti wifiMulti;
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wifi");

  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.print("WiFi connected with IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");
  }
  else {
    Serial.println("WiFi connect failed!");
  }
  
  delay(1000);

  WiFi.mode(WIFI_OFF);

  delay(1000);
}

void loop() {
}
