#include <Adafruit_Fingerprint.h>

SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const long interval = 1000;  // time interval
const int serialPin = 1;  // for commuicating bewteen arduino 
bool detect = true;
void setup()
{
  Serial.begin(9600);
  finger.begin(57600); // set the data rate for the sensor serial port

  Serial.println("\n\nAdafruit finger detect test");

  runDiagnosis();

}

void loop() {
  if(detect){getFingerprintID();}
  ////////////////////////////// retrieve message /////////////////////////
  if(Serial.available() > 0){
    char receive = Serial.read();
    if(receive == 'D'){
      getFingerprintID();
    }
  }
  // else{
  //   Serial.println("Waiting for detect Message \'D\'");
  //   delay(5000);
  // }
  
}

void runDiagnosis(){

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } 
  else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();

  if (p != FINGERPRINT_OK) {

    ////////////////////////////// send message /////////////////////////
    Serial.write('N');
    ////////////////////////////// send message /////////////////////////

    Serial.println("No match found!");
    return -1;
  } 

  // found a match!
  ////////////////////////////// send message /////////////////////////
  Serial.write('Y');

  ////////////////////////////// send message /////////////////////////

  Serial.println("\nMatch found!");
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  detect = false;
  return finger.fingerID;
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
