#include <SoftwareSerial.h> //SIMcard breakout board library

#include <Wire.h> //Needed for I2C to GPS
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //allows communication with the GPS breakout board

SFE_UBLOX_GNSS myGNSS;

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(3, 2); //SIM800L Tx & Rx is connected to Arduino #3 & #2

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing...");
  delay(1000);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  mySerial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  mySerial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  mySerial.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+ZZxxxxxxxxxx\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();

  
  Wire.begin(); // start GPS communication
  if (myGNSS.begin() == false) //check GPS wiring
  {
    Serial.println(F("u-blox GNSS module not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }
  //This will pipe all NMEA sentences to the serial port so we can see them
  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
}

void loop()
{
  updateSerial(); // update serial with SIM data
  myGNSS.checkUblox(); //See if new data is available. Process bytes as they come in.
  lockPosition();

  //delay(250); // not needed due to built in delay in updateSerial 
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

void lockPosition()
{
  long longitudeStart = myGNSS.getLongitude();
  long latitudeStart = myGNSS.getLatitude();
  delay(500);
  long newLong = myGNSS.getLongitude(); // checks position after half a second
  long newLat = myGNSS.getLatitude();  //

  if(newLong != longitudeStart || newLat != latitudeStart)
  {
    mySerial.println("Location moved"); //send text message alert
  }
}
