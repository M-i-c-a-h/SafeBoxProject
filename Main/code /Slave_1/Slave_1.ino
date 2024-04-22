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
/*
This Slave would handle keypad connections

*/
// Include Arduino Wire library for I2C
#include <Wire.h>
#include <Keypad.h>

// Define Slave I2C Address
#define SLAVE_ADDR 9

// Define Slave answer size
#define ANSWERSIZE 4

//setup keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


// Define string with response to Master;
char keyToSend= ' ';

void setup() {

  // Initialize I2C communications as Slave
  Wire.begin(SLAVE_ADDR);
  
  // Function to run when data requested from master
  Wire.onRequest(requestEvent); 
  pinMode(13, OUTPUT);
  
  // Function to run when data received from master
  Wire.onReceive(receiveEvent);
  
  // Setup Serial Monitor 
  Serial.begin(9600);
  Serial.println("I2C Slave Demonstration");
}

void receiveEvent() {

}

void requestEvent() {
  
  if(keyToSend != ' '){
    byte response = (byte) keyToSend;
    Wire.write(response);
    Serial.println("Sent");
    keyToSend= ' ';
    
  }
 
}

void loop() {
 
  char key = keypad.getKey();
  
  if (key){
    keyToSend = key;
    
    Serial.println(keyToSend);
    
  }
}