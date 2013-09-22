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
#include <AltSoftSerial.h> //When you use this you need to replace all softwareserial occurrences in the softeasy libraries with altsoftserial.
#include <SoftEasyTransfer.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <GSM.h> //Useful reference: http://purposefulscience.blogspot.de/2013/06/arduino-gsm-shield-tips.html

//Construction
AltSoftSerial mySerial2ndDuno; //tx9 rx8 unusable10
SoftEasyTransfer ET; 
GSM gsmAccess(true);        // GSM access: include a 'true' parameter for debug enabled
GPRS gprsAccess;  // GPRS access
GSMClient client;  // Client service for TCP connection

//Definitions
struct RECEIVE_DATA_STRUCTURE{
  float latitude;
  float longitude;
  // char lon_string;
  // char lat_string;
};

//Declarations
RECEIVE_DATA_STRUCTURE mydata;
char latty[20];
char longy[20];
int loop_counter=0;
char server[] = "api.xivley.com"; 
unsigned long lastConnectionTime = 0;           // last time you connected to the server, in milliseconds
boolean lastConnected = false;                  // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000;  // delay between updates to Pachube.com
boolean dataSent = false;
// ***************************************
// Setup
// ***************************************

void setup(){
  pinMode(2,INPUT);
  digitalWrite(2,HIGH);

  Serial.begin(38400); //Debug Serial
  mySerial2ndDuno.begin(9600); //Communication Serial
  ET.begin(details(mydata), &mySerial2ndDuno);  //Expect communication using data structure

  Serial.println("Initialisation complete.");
  Serial.println("Going to sleep"); 
  delay(100); //Ensure serial print has time to run  
  //Knock_Out(); //Sleep as soon as arduino setup is completed
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
    //Setup required JSON for GSM dispatch - DEBUG PRINT ATM
    //Serial.print("{\"location\": {\"disposition\": \"mobile\",\"name\": \"Car Location\",\"exposure\": \"outdoor\", \"domain\": \"physical\",\"ele\": \"0000\",\"lat\": "+lat+",\"lon\": "+lon+"}}");
    
    
      Serial.println(F("Begin GSM!"));
      START_GSM(); //Works when shielded. Needs a check on extra jump wire pins to work on breadboard. TO DO!
      Serial.println(F("End GSM!"));
  
    
    //Final step is to sleep the arduino and await for our next interrupt.
    Knock_Out();
 }
  //Addition of a delay to ensure transmission can be caught.
  Serial.println(F("Looping"));
  
  delay(250);
  
  loop_counter++;
    if (loop_counter>10)
  {
    Serial.println(F("Sleeping max loops"));
    loop_counter=0;
    Knock_Out();   
  }
}


// ***************************************
// Custom Functions
// ***************************************

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
  // Set pin 2 as interrupt and attach handler:
  attachInterrupt(0, wake, LOW);
  delay(100);

  // Choose our preferred sleep mode:
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Set sleep enable (SE) bit:
  sleep_enable();

  // Put the device to sleep:
  sleep_mode();

  // Upon waking up, sketch continues from this point.
  sleep_disable();
}

void wake ()
{
  // cancel sleep as a precaution
  sleep_disable();
  // must do this as the pin will probably stay low for a while
  detachInterrupt (0);
}  // end of wake

void START_GSM()
{
//Begin GSM work
              // connection state
          boolean notConnected = true;
        
        
while(notConnected)
  {
    if((gsmAccess.begin("")==GSM_READY) &
        (gprsAccess.attachGPRS("giffgaff.com", "giffgaff", "password")==GPRS_READY))
      notConnected = false;
    else
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }

          
      Serial.println(F("Connected to GPRS network"));
      Serial.println(F("Start Send GSM!"));
      String sendString = "sensor1";
      delay(1000);
      //SEND_DATA(sendString);
      delay(1000);
      gprsAccess.detachGPRS();
gsmAccess.shutdown();
delay(5000);

}

void SEND_DATA(String dataString)
{

  while(!dataSent)
  {
    // if there's incoming data from the net connection.
    // send it out the serial port.  This is for debugging
    // purposes only
    if (client.available())
    {
      char c = client.read();
      Serial.print(c);
    }

    // if there's no net connection, but there was one last time
    // through the loop, then stop the client
    if (!client.connected() && lastConnected)
    {
      Serial.println();
      Serial.println(F("disconnecting."));
      client.stop();
      dataSent=true;
    }

    // if you're not connected, and ten seconds have passed since
    // your last connection, then connect again and send data
    if(!client.connected() && (millis() - lastConnectionTime > postingInterval))
    {
      sendData(dataString);
    }
    // store the state of the connection for next time through
    // the loop
    lastConnected = client.connected();
  }
}

// this method makes a HTTP connection to the server
void sendData(String thisData)
{
  // if there's a successful connection:
  if (client.connect(server, 80))
  {
    Serial.println(F("connecting..."));

//    // send the HTTP PUT request:
//    client.print("PUT /v2/feeds/");
//    //client.print(FEEDID);
//    client.println(".csv HTTP/1.1");
//    client.println("Host: api.pachube.com");
//    client.print("X-ApiKey: ");
//    // client.println(APIKEY);
//    client.print("User-Agent: ");
//    // client.println(USERAGENT);
//    client.print("Content-Length: ");
//    client.println(thisData.length());
//
//    // last pieces of the HTTP PUT request
//    client.println("Content-Type: text/csv");
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request
    client.println(thisData);
  } 
  else
  {
    // if you couldn't make a connection
    Serial.println(F("connection failed"));
    Serial.println();
    Serial.println(F("disconnecting."));
    client.stop();
  }
  // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}

