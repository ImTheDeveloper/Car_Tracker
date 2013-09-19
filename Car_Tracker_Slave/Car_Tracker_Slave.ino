// ***************************************
// Car GPS Tracker System - Slave Arduino Code
// Sleep and wait for an external interrupt from master
// disable interrupt and parse data from master arduino
// Start up our gsm shield and send data to webserver
// If data sent succesfully go back to sleep and wait for next
// set of interrupts from master
// ***************************************

// ***************************************
// Library Inclusions
// ***************************************

#include <SoftEasyTransfer.h>
#include <SoftwareSerial.h>
#include <Sleep_n0m1.h>

//Construction
SoftwareSerial mySerial2ndDuno(2, 3); /// main d3 to second d3 and main d2 to second d3 for communication to master
SoftEasyTransfer ET; 
Sleep sleep;

//Definitions
struct RECEIVE_DATA_STRUCTURE{
  float latitude;
  float longitude;
  char lon_string;
  char lat_string;
};

//Declarations
RECEIVE_DATA_STRUCTURE mydata;
char latty[20];
char longy[20];
int InterruptPin = 7;

// ***************************************
// Setup
// ***************************************

void setup(){
  Serial.begin(38400); //Debug Serial
  mySerial2ndDuno.begin(9600); //Communication Serial
  
  ET.begin(details(mydata), &mySerial2ndDuno);  //Expect communication using data structure
  Serial.println("Initialisation complete.");
  Serial.println("Going to sleep");
  
  delay(100); //Ensure serial print has time to run
  
  Knock_Out(); //Sleep as soon as arduino setup is completed
}

// ***************************************
// Loop
// ***************************************


void loop(){
//Check and see if a data packet has come in.
  if(ET.receiveData()){  
       Serial.println(mydata.latitude);
       Serial.println(mydata.longitude);     
//Convert the inbound floats/doubles/ints into string ready for sending to json
       String lat = dtostrf(convertDegMinToDecDeg(mydata.latitude),10,6,latty); 
       String lon = dtostrf(convertDegMinToDecDeg(mydata.longitude),10,6,longy);
//Trim and turn the inbound data in to correct negative or positives // Need to pass in GPS.lon and GPS.lat at later date
      lat.trim();
      lon.trim();
  //      
  //       if (mydata.lon_string == 'W'){
  //        lon = "-" + lon;
  //       }
  //       
  //       if (mydata.lat_string == 'S'){
  //         lat = "-" + lat;
  //       }
//Setup required JSON for GSM dispatch - DEBUG PRINT ATM
      Serial.print("{\"location\": {\"disposition\": \"mobile\",\"name\": \"Car Location\",\"exposure\": \"outdoor\", \"domain\": \"physical\",\"ele\": \"0000\",\"lat\": "+lat+",\"lon\": "+lon+"}}");
//Final step is to sleep the arduino and await for our next interrupt.
      Knock_Out();
    }
//Addition of a delay to ensure transmission can be caught.
  delay(250);
}


// ***************************************
// Custom Functions
// ***************************************

//String gps2string (String lat, float latitude, String lon, float longitude) {
//  int dd = (int) latitude/100;
//  int mm = (int) latitude % 100;
//  int mmm = (int) round(1000 * (latitude - floor(latitude)));
//  String gps2lat = lat + int2fw(dd, 2) + " " + int2fw(mm, 2) + "." + int2fw(mmm, 3);
//  dd = (int) longitude/100;
//  mm = (int) longitude % 100;
//  mmm = (int) round(1000 * (longitude - floor(longitude)));
//  String gps2lon = lon + int2fw(dd, 3) + " " + int2fw(mm, 2) + "." + int2fw(mmm, 3);
//  String myString = gps2lat + ", " + gps2lon;
//  return myString;
//};

//// returns a string of length n (fixed-width)
//String int2fw (int x, int n) { 
//  String s = (String) x;
//  while (s.length() < n) {
//    s = "0" + s;
//  }
//  return s;
//}

  // degree-minute format to decimal-degrees
double convertDegMinToDecDeg (float degMin) {
  double min = 0.0;
  double decDeg = 0.0;
  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);
  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );
  return decDeg;
}

void Knock_Out()
{
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepInterrupt(InterruptPin,LOW); //Sleep interrupt set 
}


