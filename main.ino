#include <Wire.h>
#include <string> 
#include <SPIFFS.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"

/*
    Overall program flow:
    - program starts, initialization, sleep
    - activity detected, code in loop() runs
    - disable interrupts
    - clear interrupts
    - get state
    - add new state
    - send old state through wifi
    - renable interrupt, device sleeps
*/

const char* ssid = "";
const char* password = "";

AsyncWebServer server(80);

String hour_string;
String minute_string;
String second_string;
String finalString;

String processor(const String& var){
    //Serial.println(var);
    if(var == "HOUR"){
        return hour_string;
    }
    else if(var == "MINUTE"){
        return minute_string;
    }
    else if(var == "SECOND"){
        return second_string;
    }
    return String();
}

int ADXL345 = 0x53; // The ADXL345 sensor I2C address
int RTC = 0x68;

int state = -1; //-1 is first time startup, otherwise all other states are 
long currtime = 0;
long ref_time = 0;

// pin that the ADXL345 interrupt is connected to. High  when activity is detected, wakes up the board
// please make sure the below 2 numbers are synced
int interrupt = 15; 
gpio_num_t interrupt_GPIO = GPIO_NUM_15;

// "_t" means it is a type, not the variable itself
typedef struct state_t {
    long start_time;
    long end_time;
    int state_val;
} state_t;

state_t ** state_array; // array that stores all the state_t
int next_index = 0;     // points to the next spot in the state_array that's available
int last_index = 0;     // points to the oldest state, sends state data to wifi
int ARRAY_LENGTH = 100; // report BS material

void setup() {
    Serial.begin(9600); // Initiate serial communication for printing the results on the Serial monitor
    Wire.begin(); // Initiate the Wire library
    // configure ESP32 sleep and wakeup
    esp_sleep_enable_gpio_wakeup();
    //pinMode(interrupt, INPUT);
    gpio_wakeup_enable(interrupt_GPIO, GPIO_INTR_HIGH_LEVEL);   // report BS material

    // initialize state_array
    state_array = (state_t **)malloc(ARRAY_LENGTH * sizeof(state_t *));
    assert(state_array != NULL);
    for(int i = 0; i < ARRAY_LENGTH; i++){
        state_array[i] = (state_t *)malloc(sizeof(state_t));
        assert(state_array[i] != NULL);
        state_array[i]->state_val = -1; // -1 indicates invald, state not used
    }
    

    // setup wifi
    // Leo: the below is a direct copy from wifi_test/wifi_tester_v7/wifi_test_v7.ino setup()
        // I have no idea what this code is doing.
    {
        WiFi.begin(ssid, password);
        
        while (WiFi.status() != WL_CONNECTED){
            delay(1000);
            Serial.println("Connecting to wifi...");
        }
        finalString = "Hour: " + hour_string + "\nMinute: " + minute_string + "\nSecond: " + second_string;  
        Serial.println(finalString);
        server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/index.html", String(), false, processor);
        });
        server.on("/hour", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/plain", hour_string.c_str());
        });
        server.on("/minute", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/plain", minute_string.c_str());
        });
        server.on("/second", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/plain", second_string.c_str());
        });
        server.on("/winter", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/winter.jpg", "image/jpg");
        });
        server.begin();
    }

    //enable RTC:
    {
        Wire.beginTransmission(RTC);
        Wire.write(0); //stop Oscillator
        //Initalize to Time = 0
        Wire.write(0);
        Wire.write(0);
        Wire.write(0);
        Wire.write(0);
        Wire.write(0);
        Wire.write(0);
        Wire.write(0);
        Wire.write(0); //start 
        Wire.endTransmission();
        delay(10);
    }

    ref_time = gettime(); //moved ref_time to under RTC enable

    // configuring ADXL345 interrupt
    {
        
        
        // Set ADXL345 in measuring mode
        Wire.beginTransmission(ADXL345); // Start communicating with the device 
        Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
        // Enable measurement
        Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
        Wire.endTransmission();
        delay(10);

        //set interrupt activity threshold
        Wire.beginTransmission(ADXL345);
        Wire.write(0x24); //activity threshold register
        Wire.write(50); //(any change of over 50 in x y or z gets detected as interrupt)
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
    }
}

void loop() {
    Serial.println("Device Awake");

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
    currtime = gettime();
    Serial.print("Current Time is ");
    Serial.println(currtime);

    
    // add new state in
    if(state_array[next_index % ARRAY_LENGTH]->state_val != -1){
        Serial.println("state array full");
    }
    state_array[next_index % ARRAY_LENGTH]->state_val = state;
    state_array[next_index % ARRAY_LENGTH]->start_time = currtime - ref_time;
    state_array[next_index % ARRAY_LENGTH]->end_time = -1;
    next_index += 1;

    // send old states
    if(next_index != 1){
        state_array[last_index % ARRAY_LENGTH]->end_time = currtime - ref_time;
        //send_through_wifi(state_array[last_index % ARRAY_LENGTH]);  // Leo: not sure how the wifi code does this, need to ask Mingshi

        //serial monitor results:
        Serial.print("State ");
        Serial.print(state_array[(last_index) % ARRAY_LENGTH]->state_val);
        Serial.print( " total elapsed time was ");
        Serial.print((state_array[(last_index) % ARRAY_LENGTH]->end_time) - (state_array[(last_index) % ARRAY_LENGTH]->start_time));
        Serial.print("seconds, and New State is : ");
        Serial.println(state);
        last_index += 1;
    }
    delay(10);

    //renable interrupt, report BS material, didn't use function to reduce overhead
    Wire.beginTransmission(ADXL345);
    Wire.write(0x2E); //enable activity interrupt register
    Wire.write(20); //(20dec -> 0001 0000 binary) Bit D4 High for enabling activity interrupt
    Wire.endTransmission();
    delay(10);

    delay(1000);
    esp_light_sleep_start();
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

long gettime() {
    // Leo: I'm not sure if this is the way to get  the time from RTC
    Wire.beginTransmission(RTC);
    Wire.write(0);
    Wire.endTransmission();
    Wire.write(0);
    long seconds = 0;   // report BS material
    if (Wire.requestFrom(RTC, 7, true) == 7) {
        // Serial.print(bcdToDec(Wire.read() & 0x7f));
        // Serial.print(" seconds, ");
        // Serial.print(bcdToDec(Wire.read()));
        // Serial.print(" minutes, ");
        // Serial.print(bcdToDec(Wire.read() & 0x3f));
        // Serial.print(" hours, ");
        // Serial.print(bcdToDec(Wire.read()));
        // Serial.print(" day of week, ");
        // Serial.print(bcdToDec(Wire.read()));
        // Serial.print(" day, ");
        // Serial.print(bcdToDec(Wire.read()));
        // Serial.print(" month, ");
        // Serial.print(bcdToDec(Wire.read())+2000);
        // Serial.println(" year");
        seconds += bcdToDec(Wire.read() & 0x7f) + 1;    // seconds
        seconds += bcdToDec(Wire.read()) * 60; // minutes
        seconds += bcdToDec(Wire.read() & 0x3f) * 3600; // hours
        seconds += bcdToDec(Wire.read()) * 7 * 24 * 3600; // days of week
        seconds += bcdToDec(Wire.read()) * 24 * 3600;   // days
        seconds += bcdToDec(Wire.read()) * 24 * 3600 * 30;  // month
        seconds += (bcdToDec(Wire.read())) * 24 * 3600 * 365; // years
    }
    else{
        Serial.println(F("Failed to get RTC date and time"));
    };
    assert(seconds != 0);

    return seconds;
}

// keep commented, used to debug sleep
// void print_wakeup_reason(){
//     esp_sleep_wakeup_cause_t wakeup_reason;

//     wakeup_reason = esp_sleep_get_wakeup_cause();

//     switch(wakeup_reason)
//     {
//         case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
//         case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
//         case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
//         case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
//         case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
//         default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
//     }
// }
