#include <TFT_eSPI.h>
#include <Wire.h>
#include <string> 
#include "RTClib.h"

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

int counter = 0;

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void setup () {
 while (!Serial); 
 Serial.begin(57600);

 if (! rtc.begin()) {
   Serial.println("Couldn't find RTC");
   while (1);
 }

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
 tft.drawString(curr_time, (tft.width() / 2), (tft.height() / 2 + 16)); 
 delay(1000);
 ++counter;

 if (counter == 5){
  tft.drawString("Going to sleep...", tft.width() / 2, tft.height() / 2);
  delay(500);
  esp_deep_sleep_start();
 }  
 
}
