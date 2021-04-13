//Barrett Anderies
//Sep. 15, 2012
//This code was written to learn and pracitice using I2C protocol and hardware interrupts on the Arduino. The single and double tap registers on the ADXL345 can be read without using interrupts, 
//simplifying the code, but as mentioned, this was written to demonstrate how interrupts can be used. Get the ADXL345 datasheet here: http://www.analog.com/static/imported-files/data_sheets/ADXL345.pdf
//

#include <Wire.h>                                                                //include the Wire library (necessary for I2C and SPI protocols)
#include "ADXL345.h"                                                             //include the ADXL345 library (must be downloaded from: https://dl.dropbox.com/u/43421685/Website%20Content/ADXL345_library.zip and placed in Arduino --> libraries). Contains register addresses and functions.

//See datasheet for tuning instructions. I found the following settings to work well (for my application). Everything set to 0 is not being used.

#define THRESH_TAP 0x40                                                          //Tap threshold value
#define OFSX 0                                                                   //X-axis offset value  
#define OFSY 0                                                                   //Y-axis offset valuevalue
#define OFSZ 0                                                                   //Z-axis offset value
#define DUR 0x30                                                                 //Tap duration value
#define LATENT 0x40                                                              //Tap Latency value
#define WINDOW 0xFF                                                              //Tap window value
#define THRESH_ACT 0                                                             //Activity threshold value
#define THRESH_INACT 0                                                           //Inactivity threshold value
#define TIME_INACT 0                                                             //Inactivity time value
#define ACT_INACT_CTL 0                                                          //Axis enable control for activity and inactivity detection value
#define THRESH_FF 0                                                              //Free-fall threshold value
#define TIME_FF 0                                                                //Free-fall time value
#define TAP_AXES B00000111                                                       //Axis control for single tap/double tap value 
#define BW_RATE 0                                                                //Data rate and power mode control value
#define POWER_CTL B00001001                                                      //Power-saving features control value
#define INT_ENABLE B01100000                                                     //Interrupt enable control value
#define INT_MAP B00100000                                                        //Interrupt mapping control value
#define DATA_FORMAT B00001010                                                    //Data format control value
#define FIFO_CTL 0                                                               //FIFO control value


#define interrupt_receiver 15

byte X0, X1;                                                                     //variables to store incoming data
byte Y0, Y1;
byte Z0, Z1;
byte int_source;

boolean singleTap = false;                                                        //declare and initialize boolean variables to track tap type
boolean doubleTap = false;

ADXL345 myACC = ADXL345();                                                        //create an instance of class ADXL345() named myACC

void setup()
{
  Serial.begin(9600);                                                            //join the serial buss at 9600 baud
  pinMode(interrupt_receiver,INPUT);
  attachInterrupt(0, interrupt0, RISING);                                        //attach an interrupt on digital pin 2 (interrupt 0) for rising signal  
  attachInterrupt(1, interrupt1, RISING);                                        //attach an interrupt on digital pin 3 (interrupt 1) for rising signal
  pinMode(4, OUTPUT);                                                            //set LED pins to output
  pinMode(5, OUTPUT);
  myACC.setTHRESH_TAP(THRESH_TAP);                                               //set registers on the ADXL345 to the above defined values (see datasheet for tuning and explanations)
  myACC.setDUR(DUR);
  myACC.setLATENT(LATENT);
  myACC.setWINDOW(WINDOW);
  myACC.setTAP_AXES(TAP_AXES);
  myACC.setPOWER_CTL(POWER_CTL);
  myACC.setINT_MAP(INT_MAP);
  myACC.setDATA_FORMAT(DATA_FORMAT);
  myACC.setINT_ENABLE(INT_ENABLE);
  
}

void loop()                                                                      //main loop
{
//  if (digitalRead(interrupt_receiver)){
//    Serial.println(1);
//  }

  myACC.readDATAX(&X0, &X1);                                                     //use readDATA functions (see library) and print values
  Serial.print(X0 + (X1 << 8));
  Serial.print("\t");
  myACC.readDATAY(&Y0, &Y1);
  Serial.print(Y0 + (Y1 << 8));
  Serial.print("\t");
  myACC.readDATAZ(&Z0, &Z1);
  Serial.println(Z0 + (Z1 << 8));
  
  myACC.readINT_SOURCE(&int_source);                                             //read the interrupt source register (Important! If this register is not read every iteration the interrupts on the ADXL345 will not reset, and therefore not function)
  //Serial.println(int_source, BIN);
  
  if(singleTap)                                                                  //if single tap is detected
  {
    delay(500);                                                                  //wait half a second to see if there is another tap and if not, procedd to blink pin 4 (which will light the LED red)
    if(!doubleTap)
    {
      //Serial.println("Single Tap!");
      digitalWrite(4, HIGH);
      delay(500);
      digitalWrite(4, LOW);
    }
    else                                                                         //if a second tap is deceted during the half second wait, preceed to blink pin 5 (which will light the LED green)
    {
      //Serial.println("Double Tap");
      digitalWrite(5, HIGH);
      delay(500);
      digitalWrite(5, LOW);
    }
    singleTap = false;                                                          //reset tap values to ensure the lights stop blinking until the next tap sets either singleTap or doubleTap to true (see below)
    doubleTap = false;
  }


}

void interrupt0()                                                              //a RISING on interrupt pin 2 (interrupt 0) would mean a single tap (from how we have wired and configured the ADXL345), therefore, set singleTap to true
{
  singleTap = true;
  //Serial.println("Single Tap!");
}

void interrupt1()                                                              //a RISING on interrupt pin 3 (interrupt 1) would mean a double tap (again, from how we have wired and configured the ADXL345), therefore, set doubleTap to true
{
  doubleTap = true;
  //Serial.println("Double Tap!");
}
