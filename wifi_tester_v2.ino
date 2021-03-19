#include "Wifi.h"
#include "ESPAsyncWebServer.h"

const char* ssid = "CHANGE THIS TO NETWORK NAME";
const char* password = "CHANGE THIS TO NETWORK PASSWORD";

AsyncWebServer server(80);

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Connecting to wifi...");
  }

  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World);
  });
  server.begin();
}

void loop(){}

//I think you just put in http://#IP ADDRESS#/hello and it should show hello world?
