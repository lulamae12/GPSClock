int dstoffset = 6;
#include <SoftwareSerial.h>// Allow serial output
#include <Wire.h>// lets 328 talk to things
#include <Adafruit_GFX.h>// adafruit graphics libray to help me display lcd
#include <Adafruit_GPS.h>//gps libray for adafruit gps shei;
#include "Adafruit_LEDBackpack.h" // import i2c backpack sketch
Adafruit_7segment matrix = Adafruit_7segment();
#define TIME_24_HOUR false 

#define HOUR_OFFSET       -dstoffset
#define DISPLAY_ADDRESS   0x70//i2c addres of display// gps uses universal time so this offsets to align with central time
Adafruit_7segment clockDisplay = Adafruit_7segment();
SoftwareSerial gpsSerial(8, 7);// sheild TX to pin 8 and sheild RX to pin 7.
Adafruit_GPS gps(&gpsSerial);
const int buttonPin = 2;// pin the button is connected to.
const int ledPin = 10;

bool isDST = false; // 0 = false 1 = true
int switchState = 0;
bool gpsFix = false;
void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);//print debug in serial
  Serial.println("Clock starting!");
  clockDisplay.begin(DISPLAY_ADDRESS); //setup the display
  gps.begin(9600);// make gps transfer data at 9600 baud so its not to fast and that is the reccomended setting for the gps module
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);  // make gps only output time, loc and if it is connected by using only RMC
  
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);// update 1hz or once a second cus its a clock
  enableGPSInterrupt();// pause to process data
  switchState = digitalRead(buttonPin);// read switch data
  Serial.println("");
  if (switchState == HIGH){// if swutch is on print debug and set dst to true
    Serial.println("------------------is dst!----------------------");
     isDST = true;
     dstoffset = dstoffset - 1;// enact dst
  }  else {
    Serial.println("------------------is not dst!----------------------"); 
    isDST = false;// if switch is off print debug and set dst to false
    dstoffset = dstoffset;// reset dst
  }
  Serial.println("");

}

void loop() {
  // Loop for clock
  // if new gps data then procied
  if (gps.newNMEAreceived()) {
    gps.parse(gps.lastNMEA());//parse the hours mins and sec form gps
  }
  int hours = gps.hour + HOUR_OFFSET;  //subtracts gps hpour from utc offset so local time is gotten
  if (hours < 0) {// prtevent from going negative if time hits 0
    hours = 24+hours;
  }
  if (hours > 23) {// prevent from going over 24 hours if it is above 24
    hours = 24-hours;
  }
  int minutes = gps.minute;
  int seconds = gps.seconds;
  
  // example
  // time = 4:30 hours = 43 minutes = 5
  // 43 x 100 = 430 + 5 = 435 + 4:35
  // 
  int displayValue = hours*100 + minutes;// to get hour multioky by 100 and strip and add minutes

  
  if (!TIME_24_HOUR) {
    // prevents  from goiong past 12 in 24 hour time
    if (hours > 12) {
      displayValue -= 1200;// subtract 12 from time
    }
    // when it hits midnight (hour 0) show 12:
    else if (hours == 0) {
      displayValue += 1200;// add 12 to time
    }
  }
  clockDisplay.print(displayValue, DEC);// print to display
  Serial.print("Fix: "); Serial.print((int)gps.fix);// check if gps has fix
  Serial.println("Time: " + displayValue);
  if (gps.fix == 0){
    Serial.println("no fix!");//debug
    gpsFix = false;
    digitalWrite(ledPin, HIGH);// if no gps illuminate led
  }else{
    gpsFix = true;
    digitalWrite(ledPin, LOW);
  }
  if (gpsFix == false){
    digitalWrite(ledPin, HIGH);// if gps dont illuminate led
  }
  if (gpsFix == true){
    digitalWrite(ledPin, LOW);// if gps dont illuminate led
  }
  if (TIME_24_HOUR && hours == 0) {1// prevents from showing zeros when its not needed
  
    ;clockDisplay.writeDigitNum(1, 0);// for some reason it errors if there is no semicolon is there and idk why so i guess im leaving it there
    // pad when the minutes is a single digit
    if (minutes < 10) {
      clockDisplay.writeDigitNum(2, 0);
    }
  }
  //if (gps.fix == 0){
    //digitalWrite(ledPin, HIGH);// if no gps illuminate led
  //}
  clockDisplay.drawColon(seconds % 2 == 0); // blink colon to show seconds and its asthetic as heck
  clockDisplay.writeDisplay();// write time to display
}

SIGNAL(TIMER0_COMPA_vect) {//read gps data
  gps.read();
}

void enableGPSInterrupt() {// gps override interupt function
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

