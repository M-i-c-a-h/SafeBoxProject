/*
Group #52
Group Members 
Victor Savage - vsava3@uic.edu
Micah Olugbamila -  oolug4@uic.edu
Maryann Olugbamila - aolug3@uic.edu

PROJECT -
DEADLOCK

ABSTRACT -
This project implements a versatile
safe-box lock system that enhances
security through dual authentication
methods. Users gain access by
providing either a passcode or a
fingerprint scan, similar to modern
mobile devices. This system offers
these key features: Enhanced Security,
User Convenience, and Potential for
Customization.

*/
// This slave handles fingerprints

// Include Arduino Wire library for I2C
#include <Wire.h>
#include <EEPROM.h>//to store into adruinos memory
// Define Slave I2C Address
#define SLAVE_ADDR 12

// Define Slave answer size
#define ANSWERSIZE 4
#include <Adafruit_Fingerprint.h>


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(2, 3);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
bool authMode= false;
bool setupMode = false;
char FlagToSend = ' ';
int ID_Num = 0;

void setup() {

  // Initialize I2C communications as Slave
  Wire.begin(SLAVE_ADDR);
  
  // Function to run when data requested from master
  Wire.onRequest(requestEvent);
  pinMode(13, OUTPUT);
  
  // Function to run when data received from master
  Wire.onReceive(receiveRequest);
  
  // Setup Serial Monitor 
  Serial.begin(9600);
  Serial.println("I2C Slave Demonstration");
  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  delay(5);

  runDiagnosis();

}

// run system diagnosis on fingerprint scanner
void runDiagnosis(){
    if (finger.verifyPassword()) {
        Serial.println("Found fingerprint sensor!");
    }
    else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) { delay(1); }
    }
}
// receive data from master
void receiveRequest() {
  // Read while data received
  while (0 < Wire.available()){
      delay(3);
      char request = Wire.read();
      Serial.println(request);

      // call Enroll()
      if(request == 'A'){
          setupMode = true;
          ID_Num = EEPROM.read(0);

          Serial.println(ID_Num);
          ID_Num++; // id to be stored
      }
      // Authenticate() 
      else if(request == 'D'){
          authMode = true;
      }
      else if (request == 'C'){
        setupMode = false;
        authMode = false;
      }
      // wipe memory
      else if (request == 'X'){

        //**************** WIPE MEMORY *************
        for (int i = 0 ; i < EEPROM.length() ; i++) {
            EEPROM.write(i, 0);
        }
        //**************** WIPE MEMORY *************
        setupMode = false;
        authMode = false;
       }
  }

}
// send data to master
void requestEvent() {

    if(FlagToSend != ' '){
        byte response = (byte) FlagToSend;
        Wire.write(response);       //send flag to master
        //Serial.println("Sent");
        FlagToSend = ' ';
    }

}

void loop() {

  if (setupMode){
    // run enroll code
    while (! getFingerprintEnroll() );
  }
  
  else if(authMode){
    // run authentication code
    getFingerprintID();  // returns UserID
  }
}


///***************************************************[ ENROLL ]********************************************************
uint8_t getFingerprintEnroll() {
    
    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #");  Serial.println(ID_Num);  // send flag E
    FlagToSend = 'E';   //requestEvent();

    while (p != FINGERPRINT_OK && setupMode) { 
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Image taken"); //  send flag
                FlagToSend = 'F';   // requestEvent();
                break;
            case FINGERPRINT_NOFINGER:
                //Serial.print("."); // send flag
                //FlagToSend = 'G';   //requestEvent();
                break;
            default:
                Serial.println("Unknown error");  // send flag
                FlagToSend = 'E';   //requestEvent();
                break;
        }
    }

    // OK success!

    p = finger.image2Tz(1); // trap 6
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image converted"); // send flag
            FlagToSend = 'I';   //requestEvent();
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");  // send flag
            FlagToSend = 'J';   //requestEvent();
            return p;
        default:
            Serial.println("Unknown error");   // send flag
            FlagToSend = 'H';  // requestEvent();
            return p;
    }

    Serial.println("Remove finger"); // send flag
    FlagToSend = 'K';   //requestEvent();

    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER && setupMode) {
        p = finger.getImage();
    }
    Serial.print("ID "); Serial.println(ID_Num);


    p = -1;
    Serial.println("Place same finger again");  // send flag
    FlagToSend = 'L';  // requestEvent();

    while (p != FINGERPRINT_OK && setupMode) { 
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK: //
                Serial.println("Image taken");  // send flag
                FlagToSend = 'F';   //requestEvent();
                break;
            case FINGERPRINT_NOFINGER:
                Serial.print(".");      // send flag
                //FlagToSend = 'G';   //requestEvent();
                break;
            default:
                Serial.println("Unknown error");   // send flag
                FlagToSend = 'H';   //requestEvent();
                break;
        }
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image converted");
            FlagToSend = 'I';   //requestEvent();
            break;
        default:
            Serial.println("Unknown error");   //send flag
            FlagToSend = 'H';   //requestEvent();
            return p;
    }

    // OK converted!
    Serial.print("Creating model for #");  Serial.println(ID_Num);
    FlagToSend = 'M';  // requestEvent();

    p = finger.createModel();
    if (p == FINGERPRINT_OK) {
        Serial.println("Prints matched!");
    }
    else {
        Serial.println("Unknown error");  // send flag
        FlagToSend = 'H';  // requestEvent();
        return p;
    }

    Serial.print("ID "); Serial.println(ID_Num);
    p = finger.storeModel(ID_Num);
    if (p == FINGERPRINT_OK) {
        Serial.println("Stored!");      // send flag
        FlagToSend = 'N';   //requestEvent();
    }
    else {
        Serial.println("Unknown error");   // send flag
        FlagToSend = 'H';   //requestEvent();
        return p;
    }
    EEPROM.write(0, ID_Num);  // Store at address 0
    setupMode = false; // end enrollment send flag
    FlagToSend = '1';
    return true;
}
///***************************************************[ ENROLL ]********************************************************


///*********************************************[ AUTHENTICATION ]******************************************************
// validate fingerprint
uint8_t getFingerprintID() {
    uint8_t p = finger.getImage();
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image taken");  // return flag to master
            //FlagToSend = 'F';   //requestEvent();
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println("No finger detected"); // return flag to master
            FlagToSend = 'O';   //requestEvent();
            return p;
        default:
            Serial.println("Unknown error"); // return flag to master
            FlagToSend = 'H';   //requestEvent();
            return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image converted");  // return flag to master
            //FlagToSend = 'I';   //requestEvent();
            break;
        case FINGERPRINT_IMAGEMESS:
            //Serial.println("Image too messy");  // return flag to master
            FlagToSend = 'J';   //requestEvent();
            return p;
        default:
            Serial.println("Unknown error");   // return flag to master
            FlagToSend = 'H';  // requestEvent();
            return p;
    }

    // OK converted!
    p = finger.fingerSearch();

    if (p != FINGERPRINT_OK) {
        Serial.write('N');
        Serial.println("No match found!");  // return flag to master
        FlagToSend = 'P';   //requestEvent();
        return -1;
    }

    // found a match!
    Serial.write('Y');

    Serial.println("\nMatch found!");
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    FlagToSend = 'Q';   //requestEvent();
    authMode = false;
    return finger.fingerID;   // return flag to master
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK)  return -1;

    p = finger.fingerFastSearch();
    if (p != FINGERPRINT_OK)  return -1;

    // found a match!
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    return finger.fingerID;
}
///*********************************************[ AUTHENTICATION ]******************************************************