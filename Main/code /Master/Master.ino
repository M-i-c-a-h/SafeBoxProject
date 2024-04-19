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

int buzzerState= 0;
long buzzerStart= 0;
bool setupMode= false;
bool authMode= false;
bool passcodeMatched = false;
String setupKeyCode1="";
String setupKeyCode2="";
String stars="";
int setupFingerPrint = -1;
String authModeKeyCode="";
int authModeFingerPrint= -1;
volatile int state= 1;
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;//lcd
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);//lcd

String messages []= {"Set up your password",
                      "Setup KeyCode First",
                      "Enter KeyCode Again",
                      "Enroll FingerPrint",
                      "PassCode Matched, Scan Fingerprint",
                      "Access Granted",
                      "Authentication Mode",
                      "Enter KeyCode",
                      "PassCode Mismatched"};
String lcd_toprow= "";
String lcd_bottomrow= "";
long long lcd_start= 0; //to write to the lcd every one sec
const int buzzer = 7;
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
    // updateBottomRow(result);
   
  } 
  return result;
}
void setup() {

  // Initialize I2C communications as Master
  Wire.begin();
  
  // Setup serial monitor
  pinMode(13, OUTPUT);
  pinMode(buzzer, OUTPUT);
 
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
    clearLCD(1,1);
    lcd_toprow= messages[0];
    lcd_bottomrow= messages[1];

    //TODO: send message to fingerprint slave to run enrollment code
    
  }
  else{
    //TODO: send message to fingerprint slave to run authentication code
    authMode = true;
    clearLCD(1,1);

    lcd_toprow= messages[6];
    lcd_bottomrow= messages[7];

  }
}

void loop() {
 
  char result1 = request_Slave();//
  char result2 = request_Slave2();
  if (result1 != 0 ){
    tone(buzzer, 1000);
    buzzerState= 1;
    buzzerStart= millis();
  }
  switch(result1){
    case '#':
      clearLCD(1,1);
      lcd_toprow= messages[0];
      lcd_bottomrow= messages[1];
      setupKeyCode1= "";
      setupKeyCode2= "";
      setupFingerPrint= -1;
      setupMode= true;
      authMode= false;
      authModeKeyCode="";
      authModeFingerPrint= -1;
      break;
    default:
      break;
  }
  //TODO: bypass commands if certain commands are received from keypad

  if (setupMode){
    setupModeFunc(result1, result2);
  }
  else if(authMode){
    authModeFunc(result1, result2);
  }

  
  displayLcd(0);
  updateBuzzer();

}


void updateBuzzer(){
  if(buzzerState== 1 && millis()-buzzerStart>=500){
    buzzerState= 0;
    buzzerStart= 0;
    noTone(buzzer);
  }
  else if(buzzerState== 2 && millis()-buzzerStart>=500){
    buzzerState= 9;
    buzzerStart= 0;
    noTone(buzzer);
  }
  else if(buzzerState==9 && millis()-buzzerStart>=500){
    buzzerState= 11;
    buzzerStart= 0;
    tone(buzzer, 1000);
  }
  else if(buzzerState== 11 && millis()-buzzerStart>=500){
    buzzerState= 0;
    buzzerStart= 0;
    noTone(buzzer);
  }

  
}

void setupModeFunc(char result1, char result2){
  //TODO: add option to restart setup at anypoint during setup process
  if(setupKeyCode1.length()<4){//keyCode not set
   
    if(isdigit(result1)){
      clearLCD(0,1);
      Serial.println("here");

      // build access code from user
      setupKeyCode1 += result1;
      stars+="*";
      lcd_bottomrow=stars;
      if (setupKeyCode1.length()==4){
       clearLCD(0,1);
       lcd_bottomrow=messages[2];
       stars="";
      }
    }
    
  }
  // keyCode not confirmed
  else if(setupKeyCode2.length()<4){
    if(isdigit(result1)){
      clearLCD(0,1);
      Serial.println("here");
      
      setupKeyCode2+=result1;
      stars+="*";
      lcd_bottomrow=stars;
      if (setupKeyCode2.length()==4){
       //compare with setupKeyCode1
      clearLCD(0,1);
       stars="";
       if (setupKeyCode2==setupKeyCode1){
        lcd_bottomrow= "PassCode Match";
        passcodeMatched = true;
        WriteToFingerprint();
        lcd_bottomrow=messages[3];

       }
       else{
        lcd_bottomrow="PassCode Does not match restart";
        setupKeyCode1= "";
        setupKeyCode2= "";
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
      delay(2000);
     }
     else{
         lcd_bottomrow = "some message";
         //TODO: handle different messages
     }

  }
  else{//everything is set, exit setup mode 
      //reset all setup variables for reuse
      
      storeFourDigitValue(setupKeyCode2.toInt());
      setupMode = false;
      setupKeyCode1= "";
      setupKeyCode2= "";
      setupFingerPrint= -1;
      authMode = true;

      clearLCD(1,1);
      lcd_toprow= messages[6];
      lcd_bottomrow= messages[7];

  }
}

void clearLCD(bool top, bool bottom){
 
  if(top){
    lcd.setCursor(0, 0);
  
    lcd.print("                ");
  }
  if (bottom){
    lcd.setCursor(0, 1);
  
    lcd.print("                ");
  }
   
}
void authModeFunc(char result1, char result2){
  if (authModeKeyCode.length()<4){
    if(isdigit(result1)){
      clearLCD(0, 1);

      // build access code from user
      authModeKeyCode += result1;
      stars+="*";
      lcd_bottomrow=stars;
      if (authModeKeyCode.length()==4){
        
       //check if it matches value stored in eeprom
       int passcode= readFourDigitValue();

       Serial.println(passcode);
       if (passcode==authModeKeyCode.toInt()){
        lcd_bottomrow= messages[4];
        displayLcd(1);
        WriteToFingerprint();
       }
       else {
          authModeKeyCode= "";
          lcd_bottomrow= messages[8];
          tone(buzzer, 1000);
          buzzerState= 2;
          buzzerStart= millis();
       }
       stars="";
      }
    }
  }
  else if (authModeFingerPrint ==-1){
    updateBottomRow(result2);
    if(result2 == 'Q'){         //Two-factor authentication successful
      
      authModeFingerPrint = 1;
     

     }
     else{
         //lcd_bottomrow = "some message";
         //TODO: handle different messages
         
         if (result2=='P'){//no match found
          authMode = true;
          clearLCD(0, 1);
          lcd_toprow= messages[6];
          //lcd_bottomrow= messages[7];
          displayLcd(1);
          delay(2000);
          resetSystem();
         }
         else{
          //lcd_bottomrow="unhandled";
         }
     }
     
     //updateBottomRow(result2);
  }
}
void updateBottomRow(char flag){
  switch(flag){
      case 'E':
          lcd_bottomrow = "Waiting for valid finger to enroll new user";
          break;
      case 'F':
          lcd_bottomrow = "Image taken";
          break;
      case 'G':
          lcd_bottomrow = ".";
          break;
      case 'H':
          lcd_bottomrow = "Unknown error";
          break;
      case 'I':
          lcd_bottomrow = "Image converted";
          break;
      case 'J':
          lcd_bottomrow = "Image too messy";
          break;
      case 'K':
          lcd_bottomrow = "Remove finger";
          break;
      case 'L':
          lcd_bottomrow = "Place same finger again";
          break;
      case 'M':
          lcd_bottomrow = "Creating model for new user";
          break;
      case 'N':
          lcd_bottomrow = "Prints matched!";
          break;
      case 'O':
          lcd_bottomrow = "No finger detected";
          break;
      case 'P':
          lcd_bottomrow = "No match found!";
          break;
      case 'Q':
          lcd_toprow = "***Authentication completed***";
          lcd_bottomrow = "Access granted!!!";
          break;
      default:
      break;
  }
}
void displayLcd(bool override){
  //we can move the text here
  //TODO:move text 
  if(override || millis()-lcd_start>=500){

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
void resetSystem(){
  authModeFingerPrint ==-1;
  authModeKeyCode="";
  lcd_bottomrow = messages[7];
}
