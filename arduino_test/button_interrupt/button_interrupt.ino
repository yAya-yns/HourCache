void setup(){
  esp_sleep_enable_gpio_wakeup();
  pinMode(33, INPUT); // pin for receiving interrupt for wake up
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor
  gpio_wakeup_enable(GPIO_NUM_33, GPIO_INTR_HIGH_LEVEL);  // Hi Kelvin, here is where we declare the pin for interrupt
  Serial.println("Going to sleep now");
  delay(1000);
  esp_light_sleep_start();
}

int i;
void loop(){
  i ++;
  Serial.println(i);
  Serial.println(digitalRead(33));
  delay(1000);
  if (i%5 == 0) {
    esp_light_sleep_start();  // if no interrupt, wait until %5 = 0, restart sleep
  }
}

//void print_wakeup_reason(){
//  esp_sleep_wakeup_cause_t wakeup_reason;
//
//  wakeup_reason = esp_sleep_get_wakeup_cause();
//
//  switch(wakeup_reason)
//  {
//    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
//    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
//    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
//    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
//    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
//    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
//  }
///}
