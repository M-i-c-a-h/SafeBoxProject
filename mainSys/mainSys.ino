#include <LiquidCrystal.h>
#include <Adafruit_Fingerprint.h> //Libraries needed 
#include <SoftwareSerial.h> 
#include <Keypad.h> 

// main arduino respods to communication recieved by supporting arduinos

const long interval = 1000;  // time interval
const int serialPin = 1;  // for commuicating bewteen arduino 

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 8, d5 = 7, d6 = 6, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int buzzer = 9;   // initialize buzzer Pin

const int LED_RED = A3;  // initialize LEDs
const int LED_Yellow = A4;
const int LED_Green = A5;

const int SOL_LOCK = 4;


void setup() {

  pinMode(LED_RED, OUTPUT);       // setup LEDs
  pinMode(LED_Yellow,OUTPUT);
  pinMode(LED_Green,OUTPUT);

  pinMode(buzzer,OUTPUT);        // setup Buzzer

  Serial.begin(9600);           // initialise Serial
 
  pinMode(SOL_LOCK, OUTPUT);    // setup solenoid lock
  digitalWrite (SOL_LOCK, LOW); // close lock

  setUpScreen();      // setup and start LCD
}

void loop() {
  
  ////////////////////////////// retrieve message from KEYPAD /////////////////////////
  if(Serial.available() > 0){

    char message = Serial.read();   // information from arduino
    // handle enroll option
    if(message == 'A'){
      
    }
    // handle Bypass for debugging
    else if(message == 'B'){

    }
    // handle change password
    else if(message == 'C'){

    }
    // handle invalid pin
    else if(message == 'N'){

    }
    // handle valid pin
    else if(message == 'Y'){
      // request finger print
    }
  }
}

void setUpScreen(){

  lcd.begin(16, 2); 
  lcd.print("SECURED SYS!"); 

  lcd.blink(); 
  delay(2000); 

  lcd.setCursor(0,1); 
  lcd.print("Initializing sys"); 
  delay(2000); 
}
