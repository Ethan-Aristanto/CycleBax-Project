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
  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+ZZxxxxxxxxxx\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms.
  updateSerial();
  //mySerial.write(26);

  
  Wire.begin(); // start GPS communication
  if (myGNSS.begin() == false) //check GPS wiring
  {
    Serial.println(F("u-blox GNSS module not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }
  //This will pipe all NMEA sentences to the serial port so we can see them
  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
  delay(250);
  mySerial.println("Bike is locked!"); // Send message to user to signify the bike is locked
  updateSerial();
  mySerial.write(26); //Required to send messages to user. Think of this as saying "over" into a radio.
}

void loop()
{
  updateSerial(); // update serial with SIM data
  myGNSS.checkUblox(); //See if new data is available. Process bytes as they come in.
  lockPosition(); // keep bike position locked
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
  long longitudeStart = round(myGNSS.getLongitude()*10E-4); // Get an initial position
  long latitudeStart = round(myGNSS.getLatitude()*10E-4);
  delay(500);
  long newLong = round(myGNSS.getLongitude()*10E-4); // checks position after half a second
  long newLat = round(myGNSS.getLatitude()*10E-4);  //

  if(newLong != longitudeStart || newLat != latitudeStart) //If the position has changed
  {
    mySerial.println("AT+CMGS=\"+ZZxxxxxxxxxx\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms.
    updateSerial();
    mySerial.println("Location moved!"); //send the user a text message alert
    updateSerial(); 
    mySerial.write(26);
  }
}
