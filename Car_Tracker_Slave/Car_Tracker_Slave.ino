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

//Construction
SoftwareSerial mySerial2ndDuno(2, 3); /// main d3 to second d3 and main d2 to second d3
SoftEasyTransfer ET; 

//Definitions
struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  float latitude;
  float longitude;
};

//Declarations
RECEIVE_DATA_STRUCTURE mydata;
char latty[20];
char longy[20];

// ***************************************
// Setup
// ***************************************

void setup(){
  Serial.begin(38400);
  mySerial2ndDuno.begin(9600);
  Serial.println("Starting up..");
  ET.begin(details(mydata), &mySerial2ndDuno); 
}

// ***************************************
// Loop
// ***************************************


void loop(){
  //check and see if a data packet has come in. 
  if(ET.receiveData()){
    //this is how you access the variables. [name of the group].[variable name]
    //since we have data, we will blink it out. 
  
     Serial.println(mydata.latitude);
     Serial.println(mydata.longitude);
   
   //Convert the inbound floats/doubles/ints into string ready for sending to json
     String lat = dtostrf(convertDegMinToDecDeg(mydata.latitude),10,6,latty);
      String lon = dtostrf(convertDegMinToDecDeg(mydata.longitude),10,6,longy);
    
    //trim and turn them in to correct negative or positives // Need to pass in GPS.lon and GPS.lat at later date
        lat.trim();
      lon.trim();
      
       if (GPS.lon == 'W'){
        lon = "-" + lon;
       }
       
       if (GPS.lat == 'S'){
         lat = "-" + lat;
       }
    
    
    // Serial.print("{\"location\": {\"disposition\": \"mobile\",\"name\": \"Car Location\",\"exposure\": \"outdoor\", \"domain\": \"physical\",\"ele\": \"0000\",\"lat\": "+lat+",\"lon\": "+lon+"}}");
     
    
    }
  
  //you should make this delay shorter then your transmit delay or else messages could be lost
  delay(250);
}


// ***************************************
// Custom Functions
// ***************************************

String gps2string (String lat, float latitude, String lon, float longitude) {

  int dd = (int) latitude/100;
  int mm = (int) latitude % 100;
  int mmm = (int) round(1000 * (latitude - floor(latitude)));
  String gps2lat = lat + int2fw(dd, 2) + " " + int2fw(mm, 2) + "." + int2fw(mmm, 3);
  dd = (int) longitude/100;
  mm = (int) longitude % 100;
  mmm = (int) round(1000 * (longitude - floor(longitude)));
  String gps2lon = lon + int2fw(dd, 3) + " " + int2fw(mm, 2) + "." + int2fw(mmm, 3);
  String myString = gps2lat + ", " + gps2lon;
  return myString;
};

// returns a string of length n (fixed-width)
String int2fw (int x, int n) { 
  String s = (String) x;
  while (s.length() < n) {
    s = "0" + s;
  }
  return s;
}

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
