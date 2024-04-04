/*
  I2C Master Demo
  i2c-master-demo.ino
  Demonstrate use of I2C bus
  Master sends character and gets reply from Slave
  DroneBot Workshop 2019
  https://dronebotworkshop.com
*/

// Include Arduino Wire library for I2C
#include <Wire.h>

// Define Slave I2C Address
#define SLAVE_ADDR 9
#define SLAVE_ADDR2 12

int w = 0;
// Define Slave answer size
#define ANSWERSIZE 4
volatile int state= 1;


void setup() {

  // Initialize I2C communications as Master
  Wire.begin();
  
  // Setup serial monitor
  pinMode(11, OUTPUT);
 
  Serial.begin(9600);
  Serial.println("I2C Master Demonstration");
  digitalWrite(11, HIGH);
}

void loop() {
  //delay(2000);
  //digitalWrite(11, state);
  delay(50);
  char x = 4;

  // read data from Monitor
  while (Serial.available()>0){
    delay(3);
    x= Serial.read();
    Serial.println("From Monitor:");
    Serial.println(x);
    if (x== '0'){
      Wire.beginTransmission(SLAVE_ADDR);
  
      Wire.write(x);
      Wire.endTransmission();
        
      Wire.beginTransmission(SLAVE_ADDR);
    }
    else if(x=='1'){
      Wire.beginTransmission(SLAVE_ADDR);
  
      Wire.write(x);
      Wire.endTransmission();
        
      Wire.beginTransmission(SLAVE_ADDR);
    }
    else if(x=='5'){
      Wire.beginTransmission(SLAVE_ADDR2);
  
      Wire.write('0');
      Wire.endTransmission();
        
      Wire.beginTransmission(SLAVE_ADDR2);
    }
    else if(x=='7'){
      Wire.beginTransmission(SLAVE_ADDR2);
  
      Wire.write('1');
      Wire.endTransmission();
        
      Wire.beginTransmission(SLAVE_ADDR2);
    }
    break;
  }


  Serial.println("Write data to slave");
  
  // Write a charatre to the Slave
  // Wire.beginTransmission(SLAVE_ADDR);
  
  // Wire.write(x);
  // Wire.endTransmission();
    
  // Wire.beginTransmission(SLAVE_ADDR);
  
  // Wire.write(x);
  // Wire.endTransmission();
  Serial.println("Receive data");
  
  // Read response from Slave
  // Read back 5 characters
  Wire.requestFrom(SLAVE_ADDR2,ANSWERSIZE);
  
  // Add characters to string

  String response = "";
  String result = "";
  while (Wire.available()) {

      char b = Wire.read();
      result += b;  
      
  } 
  

  if(result == "0000"){
    digitalWrite(11, LOW);
  }
  
  Serial.print("result is ");
  Serial.println(result);
  // Print to Serial Monitor
  
  Serial.println("Receive data 2");
  
  // Read response from Slave
  // Read back 5 characters
  //Wire.flush()
  Wire.requestFrom(SLAVE_ADDR,ANSWERSIZE);
  
  // Add characters to string

  //String response = "";
  String result2 = "";
  while (Wire.available()) {

      char b = Wire.read();
      result2 += b;  
      
  } 
  

  if(result2 == "0000"){
    digitalWrite(11, HIGH);
  }
  Serial.print("result2 is ");
  Serial.println(result2);
  // Print to Serial Monitor
  
  

}