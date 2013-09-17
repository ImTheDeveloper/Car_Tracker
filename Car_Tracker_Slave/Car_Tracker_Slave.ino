#include <SoftEasyTransfer.h>

/*   For Arduino 1.0 and newer, do this:   */
#include <SoftwareSerial.h>
SoftwareSerial mySerial2ndDuno(2, 3); /// main d3 to second d3 and main d2 to second d3

/*   For Arduino 22 and older, do this:   */
//#include <NewSoftSerial.h>
//NewSoftSerial mySerial(2, 3);


//create object
SoftEasyTransfer ET; 

struct RECEIVE_DATA_STRUCTURE{
  //put your variable definitions here for the data you want to receive
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  float latitude;
  float longitude;
};

//give a name to the group of data
RECEIVE_DATA_STRUCTURE mydata;


char latty[20];
char longy[20];

void setup(){
  Serial.begin(38400);
  mySerial2ndDuno.begin(9600);
  Serial.println("Starting up..");
  //start the library, pass in the data details and the name of the serial port.
  ET.begin(details(mydata), &mySerial2ndDuno);
  
}

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
    
    }
  
  //you should make this delay shorter then your transmit delay or else messages could be lost
  delay(250);
}
