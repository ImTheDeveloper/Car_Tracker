// ***************************************
// Car GPS Tracker System - Master Arduino Code
// Get a quality fix and send lat/long to a slave
// arduino then fall asleep.
// ***************************************

// ***************************************
// Library Inclusions
// ***************************************

#include <math.h>
#include <Adafruit_GPS.h>
#include <AltSoftSerial.h>
#include <Sleep_n0m1.h>
#include <SoftEasyTransfer.h>
#include <SoftwareSerial.h>

//Construction
AltSoftSerial mySerial; //tx9 rx8 unusable10
Adafruit_GPS GPS(&mySerial);
SoftwareSerial mySerial2ndDuno(2, 3); //connect main d2 to slave d3 and main d3 to slave d2
SoftEasyTransfer ET;
Sleep sleep;

//Definitions
#define GPSECHO  false

//Declarations
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy
int GPSEnablePin = 7; //Set digital pin number to be linked to gps enable pin. This allows GPS to be powered on and off by setting HIGH/LOW
unsigned long sleepTime; //how long you want the arduino to sleep
uint32_t timer = millis();
int retry_counter; //Our counter to check how many times we loop before sleeping to conserve battery

//Specific declaration for our arduino to arduino communication
struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE SLAVE ARDUINO
  float latitude;
  float longitude;
};

SEND_DATA_STRUCTURE mydata;

// ***************************************
// Setup
// ***************************************

void setup()  
{
  //GPS Setup
  pinMode(GPSEnablePin, OUTPUT); //Sets the enable pin as output to allow voltage.
  digitalWrite(GPSEnablePin,HIGH); //Ensure GPS enabled.
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  useInterrupt(true);
  
  //Setup serial to second arduino
  mySerial2ndDuno.begin(9600);
  ET.begin(details(mydata), &mySerial2ndDuno);
  randomSeed(analogRead(0)); //Our checksum for the data connection
  
  //General Setup
  sleepTime = 10000; //Set sleep time in ms, max sleep time is 49.7 days.
  retry_counter=0; //Set to 0 for initial setup
  delay(1000);
}

// ***************************************
// Loop
// ***************************************

void loop()
{
  digitalWrite(GPSEnablePin,HIGH); //Ensure gps is disabled.
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }
  if (strstr(GPS.lastNMEA(), "RMC") && GPS.fix && GPS.fixquality) {
    //******** <----
    // if millis() or timer wraps around, we'll just reset it
    if (timer > millis())  timer = millis();
    // approximately every 2 seconds or so, print out the current stats
    if (millis() - timer > 2000) { 
      timer = millis(); // reset the timer
      digitalWrite(GPSEnablePin,LOW); //Ensure gps is disabled. // Need to test whether we move this to  ******** <--- 
      mydata.latitude = GPS.latitude;
      mydata.longitude = GPS.longitude;
      ET.sendData();
      delay(1000); //Delay added to allow for ET.sendData() to fully communicate before falling back to sleep. Slave is set to 250ms for read loop this gives us atleast 4 attempts
      Knock_Out();//Sleep
    }
  }
  
  retry_counter ++; //Add 1 to our retry counter
  if (retry_counter == 120)
  {
    digitalWrite(GPSEnablePin,LOW); //Ensure gps is disabled.
    retry_counter=0;
    Knock_Out();//Sleep
  }
  delay(1000); // Wait 1 second before hitting the loop again
}


// ***************************************
// Custom Functions
// ***************************************

void Knock_Out()
{
  //Sleep function using watchdog timer extended in most power saving mode "POWER DOWN"
  sleep.pwrDownMode();
  sleep.sleepDelay(sleepTime);
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } 
  else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}


SIGNAL(TIMER0_COMPA_vect) {
  // Interrupt is called once a millisecond, looks for any new GPS data, and stores it
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
  // writing direct to UDR0 is much much faster than Serial.print 
  // but only one character can be written at a time. 
#endif
}
