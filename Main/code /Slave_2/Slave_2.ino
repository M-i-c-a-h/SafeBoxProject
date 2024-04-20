/*
  I2C Slave Demo
  i2c-slave-demo.ino
  Demonstrate use of I2C bus
  Slave receives character from Master and responds
  DroneBot Workshop 2019
  https://dronebotworkshop.com
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

          // todo take off
          Serial.println(ID_Num);
          ID_Num++; // id to be stored
      }
      // Authenticate()  // TODO: may take this off
      else if(request == 'D'){
          authMode = true;
      }
      else if (request == 'C'){
        setupMode = false;
        authMode = false;
      }
  }

}
// send data to master
void requestEvent() {

    if(FlagToSend != ' '){
        byte response = (byte) FlagToSend;
        Wire.write(response);       //TODO: send flag to master
        //Serial.println("Sent");
        FlagToSend = ' ';
    }

}

void loop() {

  // Time delay in loop
  //delay(2000);
  if (setupMode){
      //TODO: run enroll code
      while (! getFingerprintEnroll() );   // trap 1

  }
  else if(authMode){
    //TODO:run authentication code
    
    getFingerprintID();  // returns UserID
  }
}


///***************************************************[ ENROLL ]********************************************************
uint8_t getFingerprintEnroll() {
    
    int p = -1;
    Serial.print("Waiting for valid finger to enroll as #");  Serial.println(ID_Num);  // trap 2  todo: send flag E
    FlagToSend = 'E';   //requestEvent();

    while (p != FINGERPRINT_OK && setupMode) { // trap 3
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK:
                Serial.println("Image taken"); // trap 5 todo: send flag
                FlagToSend = 'F';   //requestEvent();
                break;
            case FINGERPRINT_NOFINGER:
                //Serial.print("."); // trap 4  todo: send flag
                //FlagToSend = 'G';   //requestEvent();
                break;
            default:
                Serial.println("Unknown error");  // todo: send flag
                FlagToSend = 'H';   //requestEvent();
                break;
        }
    }

    // OK success!

    p = finger.image2Tz(1); // trap 6
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image converted"); // trap 7 todo: send flag
            FlagToSend = 'I';   //requestEvent();
            break;
        case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");  // todo: send flag
            FlagToSend = 'J';   //requestEvent();
            return p;
        default:
            Serial.println("Unknown error");   // todo: send flag
            FlagToSend = 'H';  // requestEvent();
            return p;
    }

    Serial.println("Remove finger"); // trap 8   todo: send flag
    FlagToSend = 'K';   //requestEvent();

    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER && setupMode) {
        p = finger.getImage();
    }
    Serial.print("ID "); Serial.println(ID_Num);


    p = -1;
    Serial.println("Place same finger again");  // todo: send flag
    FlagToSend = 'L';  // requestEvent();

    while (p != FINGERPRINT_OK && setupMode) { // trap 9
        p = finger.getImage();
        switch (p) {
            case FINGERPRINT_OK: // trap 10
                Serial.println("Image taken");  // todo: send flag
                FlagToSend = 'F';   //requestEvent();
                break;
            case FINGERPRINT_NOFINGER:
                Serial.print(".");      // todo: send flag
                //FlagToSend = 'G';   //requestEvent();
                break;
            default:
                Serial.println("Unknown error");   // todo: send flag
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
            Serial.println("Unknown error");   // todo: send flag
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
        Serial.println("Unknown error");  // todo: send flag
        FlagToSend = 'H';  // requestEvent();
        return p;
    }

    Serial.print("ID "); Serial.println(ID_Num);
    p = finger.storeModel(ID_Num);
    if (p == FINGERPRINT_OK) {
        Serial.println("Stored!");      // todo: send flag
        FlagToSend = 'N';   //requestEvent();
    }
    else {
        Serial.println("Unknown error");   // todo: send flag
        FlagToSend = 'H';   //requestEvent();
        return p;
    }
    EEPROM.write(0, ID_Num);  // Store at address 0
    setupMode = false; // end enrollment   todo: send flag
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
            Serial.println("Image taken");  // todo: return flag to master
            //FlagToSend = 'F';   //requestEvent();
            break;
        case FINGERPRINT_NOFINGER:
            Serial.println("No finger detected"); // todo: return flag to master
            FlagToSend = 'O';   //requestEvent();
            return p;
        default:
            Serial.println("Unknown error"); // todo: return flag to master
            FlagToSend = 'H';   //requestEvent();
            return p;
    }

    // OK success!

    p = finger.image2Tz();
    switch (p) {
        case FINGERPRINT_OK:
            Serial.println("Image converted");  // todo: return flag to master
            //FlagToSend = 'I';   //requestEvent();
            break;
        case FINGERPRINT_IMAGEMESS:
            //Serial.println("Image too messy");  // todo: return flag to master
            FlagToSend = 'J';   //requestEvent();
            return p;
        default:
            Serial.println("Unknown error");   // todo: return flag to master
            FlagToSend = 'H';  // requestEvent();
            return p;
    }

    // OK converted!
    p = finger.fingerSearch();

    if (p != FINGERPRINT_OK) {
        Serial.write('N');
        Serial.println("No match found!");  // todo: return flag to master
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
    return finger.fingerID;   // todo: return flag to master
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