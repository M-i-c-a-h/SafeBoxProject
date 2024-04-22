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

// Include Arduino Wire library for I2C
#include <Wire.h>

// Define Slave I2C Address
#define SLAVE_ADDR 9
#define SLAVE_ADDR2 12
#include <LiquidCrystal.h>
#include <EEPROM.h>//to store into adruinos memory

// Define Slaves answer size




#define ANSWERSIZE 5
int relay= 9;
int buzzerState= 0;
long buzzerStart= 0;
long openStart = 0;

bool setupMode= false;
bool authMode= false;
bool factMode = false;
bool passcodeMatched = false;
bool doorOpen = false;

String setupKeyCode1="";
String setupKeyCode2="";
String stars="";
String authModeKeyCode="";
String factoryResetCode1 = "";
String factoryResetCode2 = "";

int setupFingerPrint = -1;
int authModeFingerPrint= -1;
volatile int state = 1;
int trials = 0;
int iCursor = 0;
int iCursor1 = 0;
int iCursor2 = 0;
int count = 1;
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;//lcd
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);//lcd

String messages []= { "Set up your password",
                      "Input your KeyCode",
                      "Enter KeyCode Again",
                      "Enroll FingerPrint",
                      "PassCode Matched, Scan Fingerprint",
                      "Access Granted",
                      "Authentication Mode",
                      "Enter KeyCode",
                      "PassCode Mismatched",
                      "Door Open for 5 seconds",
                      "***Authentication completed***"
                      };
String lcd_toprow= "";
String lcd_bottomrow= "";
long long lcd_start= 0; //to write to the lcd every one sec
const int buzzer = 7;
const int buzzer2 = 8;


// setup Arduino system
void setup() {

  // Initialize I2C communications as Master
  Wire.begin();
  
  // Setup serial monitor
  pinMode(13, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(buzzer2, OUTPUT);
  //tone(buzzer2,500);
  pinMode(relay, OUTPUT);
 
  Serial.begin(9600);
  Serial.println("I2C Master Demonstration");
  digitalWrite(13, HIGH);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  digitalWrite(relay, HIGH);
  systemSetup();
}

// system looping class
void loop() {
 
  char result1 = request_Slave();
  char result2 = request_Slave2();
  if (result1 != 0 ){
    tone(buzzer, 1000);
    buzzerState= 1;
    buzzerStart= millis();
  }
  switch(result1){
    // enroll new user
    case 'A':
      lcd.clear();
      clearLCD(1,1);
     
      lcd_toprow= messages[0];
      lcd_bottomrow= messages[1];
      iCursor1=iCursor2=0;
      closeFingerprint();
      setupKeyCode1= "";
      setupKeyCode2= "";
      setupFingerPrint= -1;
      setupMode= true;
      authMode= false;
      authModeKeyCode="";
      authModeFingerPrint= -1;
      break;

      // cancel current operation
      case 'C':
      clearLCD(1,1);
      iCursor1=iCursor2=0;
      resetSystem();
      break;

      // attempt system wipe
      case 'D':
      lcd.clear();
      clearLCD(1,1);
      
      factMode = true;
      authMode = false;
      setupMode = false;
      lcd_toprow = "WARNING!!! IN FACTORY RESET MODE....";
      lcd_bottomrow = "ENTER FACTORY RESET CODE";
      iCursor1=iCursor2=0;
      
      displayLcd(1);
      break;

    default:
      break;
  }
  // bypass commands if certain commands are received from keypad

  if (setupMode){
    setupModeFunc(result1, result2);
  }
  else if(authMode){
    authModeFunc(result1, result2);
  }
  else if(factMode){
    closeFingerprint();
    FACTORY_RESET(result1);
    //setupMode = true;
  }

  // if door is open ->
  // check if time interval has elapsed -> close door
  if(doorOpen){
    // send current to solenoid
    if(millis() - openStart >= 5000){
      doorOpen = false;
      closeSesame();
    }
  }
  displayLcd(0);
  updateBuzzer();


  // if user fails 3 consecutive times -> buzzer && message
  if(trials > 2){
    if(count == 1){
      clearLCD(1,1);
      iCursor1 = iCursor2 = 0;
      lcd_toprow = "SECURITY BREACH!!!";
      displayLcd(1);
      count++;
    }
    goCrazy();
  }

}


// function resets the system
void systemSetup(){
  closeFingerprint();   // shutdown fingerprint if on
  int passcode= readFourDigitValue();

  if (passcode == -1){  //  no password on system yet
    setupMode = true;
    clearLCD(1,1);
    lcd_toprow= messages[0];
    lcd_bottomrow= messages[1];
    // send message to fingerprint slave to run enrollment code
    
  }
  else{
    // send message to fingerprint slave to run authentication code
    authMode = true;
    clearLCD(1,1);
    lcd_toprow= messages[6];
    lcd_bottomrow= messages[7];
    trials = 0;
  }
  
}


// funtion returns password saved in memory
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

// function reads in requests from Slave_1 -keypad Arduino
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

// function reads in requests from Slave_2 -fingerprint Arduino
char request_Slave2(){

  Wire.requestFrom(SLAVE_ADDR2,1);
  char result = ' ';
  if (Wire.available()) {

    char b = Wire.read();
    
    result = b;
   
  } 
  return result;
}

// function turns buzzer on until door is open
void goCrazy(){
  tone(buzzer2, 500);
  
}

// function controls door opening
// relays low current to RELAY which opens solenoid for time interval
void openSesame(){
    // send current to solenoid
  if(doorOpen){
    openStart = millis();
    // buzzer sound
    noTone(buzzer2);
    tone(buzzer,1000);
    digitalWrite(relay, LOW);
    resetSystem();
    // turn redLed off && greenLed on
  }
}


// function controls door closing
// relays high current to RELAY which closes solenoid
void closeSesame(){
    // send current to solenoid
    openStart = 0;
    lcd_toprow= messages[6];
    lcd_bottomrow= messages[7];
    clearLCD(1,1);
    iCursor1=iCursor2=0;
    displayLcd(1);
    noTone(buzzer);
    noTone(buzzer2);
    digitalWrite(relay, HIGH);
    
}

// function controls buzzer output
void updateBuzzer(){
  if(buzzerState == 1 && millis()-buzzerStart>=500){
      buzzerState= 0;
      buzzerStart= millis();
      noTone(buzzer);
    }
    else if(buzzerState== 2 && millis()-buzzerStart>=500){
      buzzerState= 9;
      buzzerStart= millis();
      noTone(buzzer);
    }
    else if(buzzerState==9 && millis()-buzzerStart>=500){
      buzzerState= 11;
      buzzerStart= millis();
      tone(buzzer, 1000);
    }
    else if(buzzerState== 11 && millis()-buzzerStart>=500){
      buzzerState= 0;
      buzzerStart= millis();
      noTone(buzzer);
    }

  
}


// function sets system for new user (Setup mode)
void setupModeFunc(char result1, char result2){

  if(setupKeyCode1.length()<4){ //keyCode not set
   
    if(isdigit(result1)){
      clearLCD(0,1);
      Serial.println("here");

      // build access code from user
      setupKeyCode1 += result1;
      stars+="*";
      lcd_bottomrow=stars;
      if (setupKeyCode1.length()==4){
        clearLCD(1,1);
        iCursor1=iCursor2=0;
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
        delay(2000);
        clearLCD(1,1);
        WriteToFingerprint();
        clearLCD(1,1);
        iCursor1=iCursor2=0;

       }
       else{
        clearLCD(1,1);
        iCursor1=iCursor2=0;
        lcd_bottomrow = "Keycode Mismatch";
        stars = "";
        displayLcd(1);
        delay(2000);
        clearLCD(0,1);
        iCursor1=iCursor2=0;
        stars = "";
        lcd_bottomrow = "Restarting.....";
        displayLcd(1);
        delay(2000);
        stars = "";
        clearLCD(0,1);
        lcd_bottomrow = messages[7];
        setupKeyCode1= "";
        setupKeyCode2= "";
       }

      }
    }
  }
  else if (setupFingerPrint == -1){   //  fingerprint not set
    updateBottomRow(result2);
      // successful enrollment
     if(result2 == '1'){
      // save result to eeprom
      setupFingerPrint = 1;
      lcd_bottomrow ="Setup complete";
      delay(2000);
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


// function clears either level of LCD screen
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


// function sets system into authentication mode
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
       if (passcode==authModeKeyCode.toInt()){//pascode match
        lcd_bottomrow= messages[4];
        displayLcd(1);
        delay(2000);
        clearLCD(1,1);
        iCursor1=iCursor2=0;
        WriteToFingerprint();

        Serial.println("fingerprint ready: ");
        authModeFingerPrint = -1; //for sanity

       }
       // passcode mismatched
       else {
          authModeKeyCode= "";
          lcd_bottomrow= messages[7];
          displayLcd(1);
          delay(1000);
          clearLCD(0,1);
          lcd_bottomrow = messages[7];
          tone(buzzer, 1000);
          buzzerState= 2;
          buzzerStart= millis();
          trials++;   // increament wrong trail count
       }
       stars="";
      }
    }
  }
  else if (authModeFingerPrint == -1){
    updateBottomRow(result2);
    if(result2 == 'Q'){         //Two-factor authentication successful
      authModeFingerPrint = 1;
      Serial.println("here");
      doorOpen = true;
      clearLCD(1,1);
      iCursor1= iCursor2=0;
      openSesame();
      trials = 0;
     }
     else{ 
  
         if (result2 == 'P'){ //no match found
          authMode = true;
          clearLCD(0, 1);
          lcd_toprow= messages[6];
          displayLcd(1);
          delay(2000);
          resetSystem();
          trials++;
         }
         
     }
  }
}

// function decodes message received from slave_2 -fingerprint scanner
void updateBottomRow(char flag){

  switch(flag){
      case 'E':
          lcd_bottomrow = "Waiting for valid finger to enroll new user";
          break;
      case 'F':
          clearLCD(1,1);
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
          lcd_toprow = messages[10];
          lcd_bottomrow = "Access granted!!!";
          break;
      default:
      break;
  }
}

// function displays new output to LCD
void displayLcd(bool override){

  if(doorOpen){ 
    lcd_toprow= messages[5];
    lcd_bottomrow= messages[9];
    
  }
  if(override || millis()-lcd_start>=500){

    lcd.setCursor(0,0);
    // if text is < 16 print
    if(lcd_toprow.length() <= 16){
      lcd.print(lcd_toprow);
    }
    // if text > 16 scroll
    else{
      lcd.print(lcd_toprow);
      scrollText((lcd_toprow + "  "),0,iCursor1);
    }
    
    lcd.setCursor(0, 1);
    // if text is < 16 print
    if(lcd_bottomrow.length() <= 16){
      lcd.print(lcd_bottomrow);
    }
    // if text > 16 scroll
    else{
      lcd.print(lcd_bottomrow);
      scrollText((lcd_bottomrow + "  "),1,iCursor2);
    }
    lcd_start=millis();
  }
}

// function turns off fingerprint scanner
void closeFingerprint(){
  Wire.beginTransmission(SLAVE_ADDR2);
  Wire.write('C');
  Wire.endTransmission();

  Wire.beginTransmission(SLAVE_ADDR2);
}


// function turns off fingerprint scanner
void wipeFingerprint(){
  Wire.beginTransmission(SLAVE_ADDR2);
  Wire.write('X');
  Wire.endTransmission();

  Wire.beginTransmission(SLAVE_ADDR2);
}


// function sends request to slave_2 -Arduino fingerprint
void WriteToFingerprint(){

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
    else{
      Wire.beginTransmission(SLAVE_ADDR2);

      Wire.write('C');
      Wire.endTransmission();

      Wire.beginTransmission(SLAVE_ADDR2);
    }
}

// function overrides system and resets
void resetSystem(){
  setupKeyCode1= "";
  setupKeyCode2= "";
  setupFingerPrint= -1;
  setupMode = false;
  authMode = false;
  factMode = false;
  systemSetup();
  authModeFingerPrint ==-1;
  authModeKeyCode="";
  stars = "";
  count = 1;
  lcd_bottomrow = messages[7]; // may not be needed
}


// control scrolling of text > 16 characters
void scrollText(String text, int level, int& iCursor) {
  int lenOfText = text.length();

  // Reset variable
  if (iCursor >= lenOfText + 16) {
    iCursor = 0;
  } 

  lcd.setCursor(0, level);

  // Clear the display
  lcd.print("                ");

  // Print the message starting from iCursor position
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, level);
    lcd.print(text[(iCursor + i) % lenOfText]);
  }

  // Increment the cursor position for next iteration
  iCursor++;

}

void FACTORY_RESET(char result1){

  if(factoryResetCode1.length()<4){ //keyCode not set
   
    if(isdigit(result1)){
      clearLCD(0,1);
      Serial.println("here");

      // build access code from user
      factoryResetCode1 += result1;
      stars+="*";
      lcd_bottomrow=stars;
      if (factoryResetCode1.length()==4){
        clearLCD(1,1);
        iCursor1=iCursor2=0;
        lcd_bottomrow=messages[2];
        stars="";
      }
    }
    
  }
  // keyCode not confirmed
  else if(factoryResetCode2.length()<4){
    if(isdigit(result1)){
      clearLCD(0,1);
      Serial.println("here");
      
      factoryResetCode2+=result1;
      stars+="*";
      lcd_bottomrow=stars;
      if (factoryResetCode2.length()==4){
       //compare with factoryResetCode1
       clearLCD(0,1);
       stars="";

       if (factoryResetCode1.equals(factoryResetCode2)){       
        Serial.println("Same");
        factoryResetCode1 = factoryResetCode2 = "";

        // send wipe message to fingerprint
        wipeFingerprint();

        //**************** WIPE MEMORY *************
        EEPROM.write(0, 255);
        EEPROM.write(1, 255);
        //**************** WIPE MEMORY *************

        resetSystem();
       }

       // ignore all commands go back to default mode
       else{
        factoryResetCode1 = factoryResetCode2 = "";
        resetSystem();
       }

      }
    }
  }

  // ******************* BE WEARY OF THIS CODE ???????????????
  else{
    clearLCD(1,1);
    factoryResetCode1 = factoryResetCode2 = "";
    resetSystem();
  }
}