#include <Wire.h>

#define I2C_SDA 21
#define I2C_SCL 22
const int interrupt_receiver = 15;

// register addresses
#define ADXL345 0x53// The ADXL345 sensor I2C address
#define Reg_THRESH_ACT 0x24
#define Reg_THRESH_INACT 0x25
#define Reg_TIME_INACT 0x26
#define Reg_INT_ENABLE 0x2E
#define Reg_INT_MAP 0x2F
// register values
#define THRESH_ACT 0x04
#define THRESH_INACT 0 
#define TIME_INACT 0
#define INT_ENABLE 0x00
#define INT_MAP 0x00


int isActivated;  // Outputs
void setup() {
  Serial.begin(115200); // Initiate serial communication for printing the results on the Serial monitor
  pinMode(interrupt_receiver,INPUT);
  
  Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device 
  
  Wire.write(Reg_THRESH_ACT); // Enable Activation
  Wire.write(THRESH_ACT); 
  
  Wire.write(Reg_INT_ENABLE); // Enable INT
  Wire.write(INT_ENABLE); 

  Wire.write(Reg_INT_MAP); // Enable INT_MAP
  Wire.write(INT_MAP); 
  
  Wire.endTransmission();
  delay(10);
}


void loop() { 
  isActivated = digitalRead(interrupt_receiver);
  if (isActivated == 1){
    Serial.println(isActivated);
  }
}
