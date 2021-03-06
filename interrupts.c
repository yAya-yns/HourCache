
# include <stdio.h>
# include "freertos/FreeRTOS.h"
# include "freertos/task.h"
# include "drive/gpio.h"
# include "sdkconfig.h"

const int PIN;  // pin looking for interrupt

#define LED 2

// interupt service routine, invoked when interrupt occurs on any pin
// IRAM_ATTR will place code in RAM, not flash memory
void IRAM_ATTR ISR() {
  digitalWrite(testPIN, HIGH);
  vTaskDelay(1000 / portTICK_PERIOD_MS);  // delays 1s
  degitalWrite(testPIN, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN, INPUT_PULLUP);
  pinMode(LED,OUTPUT);
  attachInterrupt(PIN, ISR, RISING);
}

void loop() {
  
}

