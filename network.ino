#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

void sendWeightToServer(float weight) {

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(wiiBeeSettings.ssid, wiiBeeSettings.passphrase);
    Serial.println("Connecting");
    for(int i = 0; WiFi.status() != WL_CONNECTED && i < 20; i++) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Error while connecting to the network, please check your credentials");
    return;
  }
  
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  WiFiClient client;
  HTTPClient http;

  String requestUrl = wiiBeeSettings.server;
  
  // Your Domain name with URL path or IP address with path
  http.begin(client, requestUrl.c_str());
  char requestData[512] = "";
  strcat(requestData, "log wiibeename=\""); 
  strcat(requestData, wiiBeeSettings.wiiBeeName);
  strcat(requestData, "\",weight=");
  char buffer[64];
  snprintf(buffer, sizeof buffer, "%f", weight);
  strcat(requestData, buffer);

  Serial.print("Sending POST Request : ");
  Serial.print(requestData);
  Serial.print(" to ");
  Serial.println(wiiBeeSettings.server);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Send HTTP POST request
  int httpResponseCode = http.POST(requestData);
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}
