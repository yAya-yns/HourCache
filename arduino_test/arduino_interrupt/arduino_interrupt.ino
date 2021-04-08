
# include <stdio.h>

const int PIN = 17;  // pin looking for interrupt

#define LED 15

// interupt service routine, invoked when interrupt occurs on PIN
void isr() {
  if(digitalRead(LED) == HIGH){
    digitalWrite(LED, LOW);
  }else{
    digitalWrite(LED, HIGH);
  }
//  digitalWrite(LED, HIGH);
////  vTaskDelay(1000 / portTICK_PERIOD_MS);  // delays 1s, ESP only
//  digitalWrite(LED, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  attachInterrupt(PIN, isr, RISING); // ESP only
  //attachInterrupt(digitalPinToInterrupt(PIN), isr, RISING); // arduino only
}

void loop() {

}
