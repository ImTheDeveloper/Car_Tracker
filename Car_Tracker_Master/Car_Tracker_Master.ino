// Car GPS Tracker System - Master Arduino Code
// ***************************************
// Library Inclusions
// ***************************************

#include <math.h>
#include <Adafruit_GPS.h>
#include <AltSoftSerial.h>
#include <Sleep_n0m1.h>
#include <SoftEasyTransfer.h>
#include <SoftwareSerial.h>

AltSoftSerial mySerial; //tx9 rx8 unusable10
Adafruit_GPS GPS(&mySerial);

SoftwareSerial mySerial2ndDuno(2, 3); //connect main d2 to slave d3 and main d3 to slave d2
SoftEasyTransfer ET;
  
Sleep sleep;
  
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences. 
#define GPSECHO  false

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy
unsigned long sleepTime; //how long you want the arduino to sleep
int fix_try_counter; //Initiate counter to hold value for number of fix retries
int GPSEnablePin = 7; //Set digital pin number to be linked to gps enable pin. This allows GPS to be powered on and off by setting HIGH/LOW



struct SEND_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  float latitude;
  float longitude;
};

//give a name to the group of data
SEND_DATA_STRUCTURE mydata;


void setup()  
{
    
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  pinMode(GPSEnablePin, OUTPUT); //Sets the enable pin as output to allow voltage.
  digitalWrite(GPSEnablePin,HIGH); //Ensure GPS enabled.
  //fix_try_counter=0; //start the counter from 0.
  sleepTime = 10000; //Set sleep time in ms, max sleep time is 49.7 days. Set to 30 minutes
  
 // Serial.begin(115200);
 // Serial.println("Adafruit GPS library basic test!");

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  mySerial2ndDuno.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  useInterrupt(true);

  delay(1000);
  
  //Setup serial to second arduino
   ET.begin(details(mydata), &mySerial2ndDuno);
   randomSeed(analogRead(0));
}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

uint32_t timer = millis();
void loop()                     // run over and over again
{
  digitalWrite(GPSEnablePin,HIGH); //Ensure gps is disabled.
  

  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }



  if (strstr(GPS.lastNMEA(), "RMC")) && GPS.fix && GPS.fixquality) {
//******** <----
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    
      digitalWrite(GPSEnablePin,LOW); //Ensure gps is disabled. // Need to test whether we move this to  ******** <----
    
      
  
      
      mydata.latitude = GPS.latitude;
      mydata.longitude = GPS.longitude;
      ET.sendData();

      delay(1000); //Delay added to allow for ET.sendData() to fully communicate before falling back to sleep.

        //Sleep
          sleep.pwrDownMode(); //set sleep mode
          sleep.sleepDelay(sleepTime); //sleep for: sleepTime
    
  }
  
}
  
}


// ***************************************
// Custom Functions
// ***************************************


