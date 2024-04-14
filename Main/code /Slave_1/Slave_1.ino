/*
  I2C Slave Demo
  i2c-slave-demo.ino
  Demonstrate use of I2C bus
  Slave receives character from Master and responds
  DroneBot Workshop 2019
  https://dronebotworkshop.com
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
char keyToSend=' ';

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

  // Read while data received
  // while (0 < Wire.available()) {
  //   delay(3);
  //   char x = Wire.read();
  //   answer += x;
  //   Serial.println(answer);

  //   if (x=='1'){
  //     digitalWrite(13, HIGH);

  //   }
  //   else if (x=='0'){
  //     digitalWrite(13, LOW);

  //   }
  //   else {
  //     Serial.println(x);
  //   }
  
  
  // // Print to Serial Monitor
  // Serial.println("Receive event");
  // }
}

void requestEvent() {

  // // Setup byte variable in the correct size
  // byte response[ANSWERSIZE];
  
  // // Format answer as array
  // for (byte i=0;i<ANSWERSIZE;i++) {
  //   response[i] = (byte)answer.charAt(i);
  // }
  
  // // Send response back to Master
  // if(answer.length() == 4){
  //   Wire.write(response,sizeof(response));
  //   answer = "";
  // }
  
  if(keyToSend != ' '){
    byte response= (byte) keyToSend;
    Wire.write(response);
    Serial.println("Sent");
    keyToSend= ' ';
    
  }
 
  
  // Print to Serial Monitor
}

void loop() {

  // Time delay in loop
  //delay(2000);
 
  char key = keypad.getKey();
  
  if (key){
    keyToSend= key;
    
    Serial.println(keyToSend);
    
  }
}