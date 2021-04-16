/*
    Arduino and ADXL345 Accelerometer Tutorial
     by Dejan, https://howtomechatronics.com
*/
#include <Wire.h>  // Wire library - used for I2C communication
int ADXL345 = 0x53; // The ADXL345 sensor I2C address
int RTC = 0x68;

int state = -1; //-1 is first time startup, otherwise all other states are 
int currtime = 0;

int interrupt = 15;

void setup() {
  Serial.begin(9600); // Initiate serial communication for printing the results on the Serial monitor

  pinMode(interrupt, INPUT);
  
  if (state == -1) {//First Time Setup:
      Wire.begin(); // Initiate the Wire library
      // Set ADXL345 in measuring mode
      Wire.beginTransmission(ADXL345); // Start communicating with the device 
      Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
      // Enable measurement
      Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
      Wire.endTransmission();
      delay(10);

      // Change Range to +- 4g
      //Wire.beginTransmission(ADXL345);
      //Wire.write(0x31);
      //Wire.write(1); //(1dec -> 000 0001 binary) Bit D0 High for +-4g range, additional overhead for interrupt
      //Wire.endTransmission();
      //delay(10);

      //set interrupt activity threshold
      Wire.beginTransmission(ADXL345);
      Wire.write(0x24); //activity threshold register
      Wire.write(35); //(any change of over 50 in x y or z gets detected as interrupt)
      Wire.endTransmission();
      delay(10);

      //enable relative interrupts on all axis
      Wire.beginTransmission(ADXL345);
      Wire.write(0x27);
      Wire.write(240); //( -> 1111 0000 binary) Bit D7-4 to High to enable relative thresholding on all axis
      Wire.endTransmission();
      delay(10);

      //enable interrupts to trigger INT1
      Wire.beginTransmission(ADXL345);
      Wire.write(0x2F);
      Wire.write(0); //
      Wire.endTransmission();
      delay(10);
      
      //Enable Interrupts on ADXL
      Wire.beginTransmission(ADXL345);
      Wire.write(0x2E); //enable activity interrupt register
      Wire.write(20); //(20dec -> 0001 0000 binary) Bit D4 High for enabling activity interrupt
      Wire.endTransmission();
      delay(10);
    
      //enable RTC:
//      Wire.beginTransmission(RTC);
//      Wire.write(0); //stop Oscillator
//      //Initalize to Time = 0
//      Wire.write(0);
//      Wire.write(0);
//      Wire.write(0);
//      Wire.write(0);
//      Wire.write(0);
//      Wire.write(0);
//      Wire.write(0);
//      Wire.write(0); //start 
//      Wire.endTransmission();
//      delay(10);
  }
  
 
}
void loop() {
  int buttonState = 0;
  buttonState = digitalRead(interrupt);
  Serial.println(buttonState);
  if (buttonState == HIGH){
    Serial.println("Interrupt!");
    //disable interrupt
    Wire.beginTransmission(ADXL345);
    Wire.write(0x2E); //enable activity interrupt register
    Wire.write(0); //(20dec -> 0001 0000 binary) Bit D4 High for enabling activity interrupt
    Wire.endTransmission();
    delay(10);
    
    //clear interrupt
    Wire.beginTransmission(ADXL345);
    Wire.write(0x30); //read interrupt source register
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345, 1,true); //clear the interrupt
    Wire.endTransmission();
    delay(10);

    //get state
    delay(100); //wait for cube to be replaced
    state = getstate();
    Serial.print("New State is : ");
    Serial.println(state);
    //currtime = gettime();
    delay(10);
    //renable interrupt
    Wire.beginTransmission(ADXL345);
    Wire.write(0x2E); //enable activity interrupt register
    Wire.write(20); //(20dec -> 0001 0000 binary) Bit D4 High for enabling activity interrupt
    Wire.endTransmission();
    delay(10);
    
  }
  delay(1000);
}

int b2signedint(int b){
  if ((b >> 10)&1 == 1) {
    return -((~b + 1)&1023);
  }
  return b;
}

byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
};

byte decToBcd(byte val){
// Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
};

int getstate(){
  int X_out, Y_out, Z_out;
  int state = -1;
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  X_out = ( Wire.read()| Wire.read() << 8); // X-axis value
  X_out = b2signedint(X_out);
  //X_out = X_out/256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  Y_out = ( Wire.read()| Wire.read() << 8); // Y-axis value
  Y_out = b2signedint(Y_out);
  //Y_out = Y_out/256;
  Z_out = ( Wire.read()| Wire.read() << 8); // Z-axis value
  Z_out = b2signedint(Z_out);
  if (abs(X_out) > abs(Z_out)){
    if (abs(X_out) > abs(Y_out)) {
      if (X_out > 0) {
        state = 0;
      }
      else {
        state = 1;
      }
    }
    else {
      if (Y_out > 0) {
        state = 2;
      }
      else {
        state = 3;
      }
    }
  }
  else {
    if (abs(Z_out) > abs(Y_out)) {
      if (Z_out > 0) {
        state = 4;
      }
      else {
        state = 5;
      }
    }
    else {
      if (Y_out > 0) {
        state = 2;
      }
      else {
        state = 3;
      }
    }
  }
  return state;
}

int gettime() {
  Wire.beginTransmission(RTC);
  Wire.write(0);
  Wire.endTransmission();
  Wire.write(0);
  if (Wire.requestFrom(RTC, 7, true) == 7) {
    Serial.print(bcdToDec(Wire.read() & 0x7f));
    Serial.print(" seconds, ");
    Serial.print(bcdToDec(Wire.read()));
    Serial.print(" minutes, ");
    Serial.print(bcdToDec(Wire.read() & 0x3f));
    Serial.print(" hours, ");
    Serial.print(bcdToDec(Wire.read()));
    Serial.print(" day of week, ");
    Serial.print(bcdToDec(Wire.read()));
    Serial.print(" day, ");
    Serial.print(bcdToDec(Wire.read()));
    Serial.print(" month, ");
    Serial.print(bcdToDec(Wire.read())+2000);
    Serial.println(" year");
  }
  else{
    Serial.println(F("Failed to get RTC date and time"));
  };
  
  return 1;
}
    
