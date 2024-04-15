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
#include <LiquidCrystal.h>
#include <EEPROM.h>//to store into adruinos memory
// Define Slaves answer size




#define ANSWERSIZE 5


bool setupMode= false;
bool authMode= false;
bool passcodeMatched = false;
String setupKeyCode1="";
String setupKeyCode2="";
String stars="";
int setupFingerPrint = -1;

volatile int state= 1;
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;//lcd
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);//lcd
String messages []= {"Set up your password",
                      "Setup KeyCode First",
                      "Enter KeyCode Again",
                      "Enroll FingerPrint"};
String lcd_toprow= "";
String lcd_bottomrow= "";
long long lcd_start= 0; //to write to the lcd every one sec

void storeFourDigitValue(int value) {
  // Ensure value is within the four-digit range
  if (value < 0 || value > 9999) {
    return;  // Handle error if necessary
  }

  byte highByte = (value >> 8) & 0xFF;  // Extract higher-order byte
  byte lowByte = value & 0xFF;          // Extract lower-order byte

  EEPROM.write(0, highByte);  // Store at address 0
  EEPROM.write(1, lowByte);  // Store at address 1
}

int readFourDigitValue() {
  byte highByte = EEPROM.read(0);
  byte lowByte = EEPROM.read(1);
  if (highByte == 255 && lowByte == 255) {
        return -1; // Or another value that signifies "not stored"
    }
  return (highByte << 8) | lowByte; // Reconstruct the original value
}

// keypad Arduino
char request_Slave(){
  //this slave should return keypad related data
  Wire.requestFrom(SLAVE_ADDR,1);
  char result = ' ';
  if (Wire.available()) {

    char b = Wire.read();

    return b;
   
  } 
  return result;
}

char request_Slave2(){
  //this slave should return fingerprint related data

  Wire.requestFrom(SLAVE_ADDR2,1);
  char result = ' ';
  if (Wire.available()) {

    char b = Wire.read();
    
    result = b;
   
  } 
  return result;
}
void setup() {

  // Initialize I2C communications as Master
  Wire.begin();
  
  // Setup serial monitor
  pinMode(13, OUTPUT);
 
  Serial.begin(9600);
  Serial.println("I2C Master Demonstration");
  digitalWrite(13, HIGH);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  int passcode= readFourDigitValue();
  if (passcode == -1){//no password on system yet
    setupMode = true;
    lcd.clear();
    lcd_toprow= messages[0];
    lcd_bottomrow= messages[1];

    //TODO: send message to fingerprint slave to run enrollment code
    
  }
  else{
    //TODO: send message to fingerprint slave to run authentication code
    authMode = true;
    lcd.clear();
//    lcd_toprow= messages[2];
//    lcd_bottomrow=messages[3];
  }
}

void loop() {
  //example_operation();
 
  char result1 = request_Slave();//
  char result2 = request_Slave2();

  if (setupMode){
    setupModeFunc(result1, result2);
  }
  else if(authMode){
    authModeFunc(result1, result2);
  }



  displayLcd();
  
  

}




void setupModeFunc(char result1, char result2){
  //TODO: add option to restart setup at anypoint during setup process
  if(setupKeyCode1.length()<4){//keyCode not set
   
    if(isdigit(result1)){
      lcd.clear();
      Serial.println("here");

      // build access code from user
      setupKeyCode1 += result1;
      stars+="*";
      lcd_bottomrow=stars;
      if (setupKeyCode1.length()==4){
        lcd.clear();
       lcd_bottomrow=messages[2];
       stars="";
      }
    }
    
  }
  // keyCode not confirmed
  else if(setupKeyCode2.length()<4){
    if(isdigit(result1)){
      lcd.clear();
      Serial.println("here");
      
      setupKeyCode2+=result1;
      stars+="*";
      lcd_bottomrow=stars;
      if (setupKeyCode2.length()==4){
       //compare with setupKeyCode1
       lcd.clear();
       if (setupKeyCode2==setupKeyCode1){
        lcd_bottomrow= "PassCode Match";
        passcodeMatched = true;
        WriteToFingerprint();
        lcd_bottomrow=messages[3];
        stars="";
       }
       else{
        lcd_bottomrow="PassCode Does not match restart";

       }

      }
    }
  }
  else if (setupFingerPrint == -1){//fingerprint not set
      // successful enrollment
     if(result2 == '1'){
      //TODO: save result to eeprom
      setupFingerPrint = 1;
      lcd_bottomrow ="Setup complete";
     }
     else{
         lcd_bottomrow = "some message";
     }
     // unsuccessful
//     else if(result2 == '-1'){
//      lcd_bottomrow= "Try Again";
//     }
    
  }
  else{//everything is set, exit setup mode 
      //reset all setup variables for reuse
      //TODO: save keycode value to eeprom
      setupMode = false;
      setupKeyCode1= "";
      setupKeyCode2= "";
      setupFingerPrint= -1;

      authMode = true;

      //TODO: send message to fingerprint slave to run authentication code

  }

  

  
}

void authModeFunc(char resultA, char resultB){

}
void displayLcd(){
  //we can move the text here
  //TODO:move text 
  if(millis()-lcd_start>=1000){

    
    lcd.setCursor(0,0);
    lcd.print(lcd_toprow);
    lcd.setCursor(0, 1);
  
    lcd.print(lcd_bottomrow);
    lcd_start=millis();
  }
}

void WriteToFingerprint(){
    delay(2000);


    if (setupMode){
        Wire.beginTransmission(SLAVE_ADDR2);

        Wire.write('A');
        Wire.endTransmission();

        Wire.beginTransmission(SLAVE_ADDR2);
    }
    else if(authMode ){
        Wire.beginTransmission(SLAVE_ADDR2);

        Wire.write('D');
        Wire.endTransmission();

        Wire.beginTransmission(SLAVE_ADDR2);
    }
}

void example_operation(){
  //delay(2000);
  //digitalWrite(11, state);
  delay(2000);
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
    digitalWrite(13, LOW);
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
    digitalWrite(13, HIGH);
  }
  Serial.print("result2 is ");
  Serial.println(result2);
  // Print to Serial Monitor

}