
/*
 * Project Idea: Door Lock Security System
 *    - the idea here is to create a door lock security system with
 *      keypad entry for the password. If the password entered is 
 *      wrong, the alarm will be triggered. The alarm could be 
 *      disabled by entering the admin password.
 *      
 *      If the password is correct, it will open the door lock then
 *      after sometime, will automatically close the door lock.
 *      
 *      Please feel free to modify this source code to adapt 
 *      to your specific application. Please do not forget to SUBSCRIBE.
 *      
 * Author: George Bantique (TechToTinker) @ July 23, 2020
 */

// Place include libraries here:
#include "LiquidCrystal.h"
#include "Keypad.h"
#include "Servo.h"

// Pin definitions here:
#define RED_LED A0
#define BLU_LED A1
#define BUZ_PIN A2
#define SRV_PIN A3

// Global variables here:
char* password = "1234";  // User password
char* admnpass = "9876";  // Admin password

uint8_t keyPos = 0;       // Key position for password entry
uint8_t invalid_cnt = 0;  // Invalid entry counter

bool isAlarmed = false;   // Alarmed flag, use for alarming sound and flashing light

uint8_t currStatus = 0;   // This is for the current requested state
uint8_t prevStatus = 0;   // This holds the previous state, #### INITIALIZED THIS TO VALUE 1 #####

const byte ROWS = 4;      // 4 keypad rows
const byte COLS = 4;      // 4 keypad columns

char keys[ROWS][COLS] = { // keypad key array
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {7, 6, 5, 4}; // pin assignments for the keypad
byte colPins[COLS] = {3, 2, 1, 0}; 

// Create the objects here:
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//**
LiquidCrystal lcd (13, 12, 11, 10, 9, 8); // pins of the LCD. (RS, E, D4, D5, D6, D7)
//**
Servo myservo;

// Function prototypes here:
void doAlarm();
void manageKeypad();
void manageStatus();

void setup(){
  lcd.begin(16, 2);         // Initialized the lcd as 16 characters by 2 lines
  pinMode(RED_LED, OUTPUT); // Set the pin directions
  pinMode(BLU_LED, OUTPUT);
  pinMode(BUZ_PIN, OUTPUT);
  myservo.attach(SRV_PIN);  // attaches the servo on pin to the servo object
  myservo.write(90);        // set initial angle
  manageStatus();           // Set initial lcd display
} // end of void setup()
  
void loop() {
  manageKeypad();           // Manage keypad inputs
  manageStatus();           // Respond according to keypad key presses
  if (isAlarmed) {          // If alarm is triggered
    doAlarm();              // Make an alarm notification
  } 
} // end of void loop()

void doAlarm() {
  tone(BUZ_PIN, 1000);        // Send 1KHz sound signal...
  digitalWrite(RED_LED, HIGH);
  digitalWrite(BLU_LED, LOW);
  delay(150);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLU_LED, HIGH);
  noTone(BUZ_PIN);
  delay(50);
} // end of void doAlarm()

void manageStatus() {
  if ( currStatus != prevStatus ) {   // check if the status is different from previous
                                      // this is to avoid unnecessary lcd updating same display
    switch(currStatus) {
      case 0:                         // default screen
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("    Welcome     ");
        lcd.setCursor(0,1);
        lcd.print(" Enter password ");
        digitalWrite(RED_LED, HIGH);
        digitalWrite(BLU_LED, LOW);
        prevStatus = currStatus;
        currStatus = 0;
        break;
      case 1:                         // invalid entry
        lcd.clear();
        lcd.print(" Invalid entry  ");
        delay(1000);
        prevStatus = currStatus;
        currStatus = 0;
        break;
      case 2:                         // valid entry
        digitalWrite(BLU_LED, HIGH);
        delay(100);
        digitalWrite(BLU_LED, LOW);
        prevStatus = currStatus;
        currStatus = 0;
        break;
      case 3:                         // entry verified
        lcd.clear();
        lcd.print(" Entry verified ");
        digitalWrite(BLU_LED, HIGH);
        delay(1000);
        prevStatus = currStatus;
        currStatus = 4;
        break;
      case 4:                         // notification for opening door
        lcd.clear();
        lcd.print(" Opening lock ");
        isAlarmed = false;
        tone(BUZ_PIN, 1000); // Send 1KHz sound signal...
        delay(2000);
        noTone(BUZ_PIN);
        for (int pos = 90; pos >= 0; pos--) {
          myservo.write(pos);
          delay(10);
        }
        prevStatus = currStatus;
        currStatus = 5;
        break;
      case 5:                         // unlocking the door lock
        lcd.clear();
        lcd.print(" Door open ");
        digitalWrite(RED_LED, LOW);
        delay(5000);
        prevStatus = currStatus;
        currStatus = 6;
        break;
      case 6:                         // warning, door will close
        lcd.clear();
        lcd.print(" Door closing ");
        for (int i=0; i < 20; i++) {
          tone(BUZ_PIN, 1000);
          delay(150);
          noTone(BUZ_PIN);
          delay(50);
        }
        prevStatus = currStatus;
        currStatus = 7;
        break;
      case 7:                         // locking the door lock
        digitalWrite(RED_LED, HIGH);
        digitalWrite(BLU_LED, LOW);
        for (int pos = 0; pos <= 90; pos++) {
          myservo.write(pos);
          delay(10);
        }
        lcd.clear();
        lcd.print(" Door Close ");
        delay(2000);
        prevStatus = currStatus;
        currStatus = 9;
        break;
      case 8:                         // Continues alarm
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("    Alarmed     ");
        lcd.setCursor(0,1);
        lcd.print("Enter admin pass");
        prevStatus = currStatus;
        break;
      case 9:                         // Promotional message :)
        lcd.clear();
        lcd.print("  TechToTinker  ");
        lcd.setCursor(0,1);
        lcd.print(" - Project Ideas");
        delay(3000);
        currStatus = 0;
      default:
        break;
    }
  }
} // end of void manageStatus

void manageKeypad() {
  char key = keypad.getKey();         // Get the key press
  
  if (key){ 
    if (!isAlarmed) {                 // Currently in no alarm status
      if (key == password[keyPos]) {  // user password entered is still correct
        currStatus = 2;               // valid entry
        keyPos = keyPos + 1;          // increment the password key position
      } else {                        // user password entered is incorrect
        keyPos = 0;                   // reset key position, [possibility to change this to increase security
                                      // like press something (i.e * or #) to reset counter   
        currStatus = 1;               // invalid entry
        invalid_cnt = invalid_cnt + 1;// increment invalid counter
        if (invalid_cnt == 3) {       // if 3 times invalid entry, set an alarm
          currStatus = 8;             // alarmed
          isAlarmed = true;
        }
      }
      if (keyPos == 4) {              // user password entered is correct
        keyPos = 0;                   // reset password key position
        currStatus = 3;               // entry verified
      }
    } else {                          // Currently in alarmed status
      // currently alarming, enter admin password to disable alarm
      if (key == admnpass[keyPos]) {  // admin password entry is still correct
        invalid_cnt = 0;              // reset invalid counter
        keyPos = keyPos + 1;          // increment the password key position
      } else {                        // admin password entered is incorrect
        keyPos = 0;                   // reset key position, [possibility to change this to increase security
                                      // like press something (i.e * or #) to reset counter  
        invalid_cnt = invalid_cnt + 1;// increment invalid counter
      }
      if (keyPos == 4) {              // admin password entered is correct
        keyPos = 0;                   // reset password key position
        currStatus = 0;               // reset the current status
        isAlarmed = false;            // disabled current alarm
      }
    }
  }
} // end of void manageKeypad
