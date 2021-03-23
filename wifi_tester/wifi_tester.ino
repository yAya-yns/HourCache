#include <TFT_eSPI.h>
#include <Wire.h>
#include <string> 
#include "RTClib.h"

#include <WiFi.h>
#include "ESPAsyncWebServer.h"

#include <SPIFFS.h>

//Libraries for LoRa
#include <SPI.h>
#include <LoRa.h>

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

const char* ssid     = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

int counter = 0;

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

String hour;
String minute;
String second;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void connectWiFi(){
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "HOUR"){
    return hour;
  }
  else if(var == "MINUTE"){
    return minute;
  }
  else if(var == "SECOND"){
    return second;
  }
  return String();
}

void setup () {
 while (!Serial); 
 Serial.begin(57600);
 SPIFFS.begin(true);
 connectWiFi();
 
 if (! rtc.begin()) {
   Serial.println("Couldn't find RTC");
   while (1);
 }
 if(!SPIFFS.begin()){
   Serial.println("An Error has occurred while mounting SPIFFS");
   return;
 }
  // Route for root / web page
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send(SPIFFS, "/index.html", String(), false, processor);
 });
 server.on("/hour", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send_P(200, "text/plain", hour.c_str());
 });
 server.on("/minute", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send_P(200, "text/plain", minute.c_str());
 });
 server.on("/second", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send_P(200, "text/plain", second.c_str());
 });
 server.on("/winter", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send(SPIFFS, "/winter.jpg", "image/jpg");
 });
 server.begin();

 if (! rtc.isrunning()) {
   Serial.println("RTC is NOT running!");
   // following line sets the RTC to the date & time this sketch was compiled
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   // This line sets the RTC with an explicit date & time, for example to set
   // January 21, 2014 at 3am you would call:
   // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
 }

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(0, 0);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //enable sleep every 5 seconds
}

void loop () {
 DateTime now = rtc.now();
 tft.fillScreen(TFT_BLACK);
 tft.drawString("Current Time is", tft.width() / 2, tft.height() / 2 - 16);
 String curr_time = String(now.hour()) + String(":") + String(now.minute()) + String(":") + String(now.second());
 hour = String(now.hour());
 minute = String(now.minute());
 second = String(now.second());
 
 tft.drawString(curr_time, (tft.width() / 2), (tft.height() / 2 + 16)); 
 delay(1000);
 ++counter;

 if (counter == 5){
  tft.drawString("Going to sleep...", tft.width() / 2, tft.height() / 2);
  delay(500);
  esp_deep_sleep_start();
 }  
 
}
