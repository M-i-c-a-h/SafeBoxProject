#include <LiquidCrystal.h>
#include <Adafruit_Fingerprint.h> //Libraries needed 
#include <SoftwareSerial.h> 
#include <Keypad.h> 

const int rs = 14, en =15, d4 = 16, d5 = 17, d6 = 18, d7 = 19;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); 
SoftwareSerial mySerial(2, 3); //Fingerprint sensor wiring RX 3, TX2
const byte numRows= 4; //number of rows on the keypad
const byte numCols= 4; //number of columns on the keypad
char keypressed; 
char code[]={'1','2','3','4'}; //Passcode needed you can change it just keep the format "4 digits as char array" 
char c[4]; //Array to get the code fromtheuser
int ij; 
//keymap defines the key pressed according to the row and columns just as appears on the keypad 
char keymap[numRows][numCols]= 
{ 
{'1', '2', '3', 'A'}, 
{'4', '5', '6', 'B'}, 
{'7', '8', '9', 'C'}, 
{'*', '0', '#', 'D'} 
};

byte rowPins[numRows] = {12,11,10,9}; //Rows 0 to 3 //if you modifyyour pins you should modify this too 
byte colPins[numCols]={8,7,6,5}; //Columns 0 to 3 
//initializes an instance of the Keypad class 
Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols); 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); 
uint8_t id; 
#define lockOutput 13 
void setup() 
{ 
Serial.begin(9600); 
pinMode(lockOutput, OUTPUT); 
finger.begin(57600); 
lcd.begin(16, 2); 
// Print a message to the LCD. 
lcd.print("SMART LOCK SYS!"); 
lcd.blink(); 
delay(2000); 
lcd.setCursor(0,1); 
lcd.print("Initializing sys"); 
delay(2000); 
digitalWrite (lockOutput, LOW ); } 

void loop() 
{ 
  getFingerprintIDez(); //Waiting for a fingerprint toscan if it's valid or not 
  keypressed = myKeypad.getKey(); //Reading the buttons typedby the keypad
  if(keypressed == 'A'){ //If it's 'A' it triggers "Addingnew template" sequence 
  ij=0; //ij is set to 0 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.write("ENROLL NEW USER"); 
  lcd.setCursor(0, 1); 
  lcd.write("Read mode..."); 
  delay(4000); 
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("Enter Password"); 
  getPassword(); //Getting the passcodefunction 
  if(ij==4){ //If the passcode is correct we can start enrolling new finger template 
  Enrolling(); //Enrolling function 
  delay(2000); 
  lcd.clear(); 
  } 
  else{ //If the code is wrong wecan't add new users (templates) 
  lcd.setCursor(0,0); 
  lcd.print("Wrong code"); 
  delay(2000); 
  lcd.clear(); 
  } 
} 
if(keypressed == 'B') //As the keypad is meant to stay insidethe 'B' button opens the door lock 
{ //Because the systemI usedneedsan electrical signal to open 
lcd.clear(); 
lcd.setCursor(0, 0); 
lcd.write("SYSTEM BYPASS!"); 
lcd.blink(); 
lcd.setCursor(0, 1);

lcd.write("Instant Unlock!"); 
lcd.blink(); 
delay(4000); 
ij=0; //ij is set to 0 lcd.clear(); 
lcd.setCursor(0,0); 
lcd.print("Enter Password"); 
getPassword(); //Getting the passcode function 
if(ij==4) 
{ 
  digitalWrite (lockOutput, HIGH ); 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.write("SYSTEM BYPASS!"); 
  lcd.blink(); 
  lcd.setCursor(0, 1); 
  lcd.write("Door is now open!"); 
  lcd.blink(); 
  delay(5000); //Opening delay 
  digitalWrite (lockOutput, LOW ); 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.write("SYSTEM BYPASS!"); 
  lcd.blink(); 
  lcd.setCursor(0, 1); 
  lcd.write("Door is closed!"); 
  delay(2000); 
} 
else{ //If the code is wrong we can't addnewusers (templates) 
lcd.setCursor(0,0); 
lcd.print("Wrong code"); 
delay(2000); 
lcd.clear(); 
lcd.clear(); 
}

} 
lcd.clear(); 
lcd.setCursor(0, 0); 
lcd.write("PLACE FINGER NOW!"); 
lcd.setCursor(0, 1); 
lcd.write("Read mode..."); 
delay(50); 
} 
//Getting password (code) function, it gets the characters typed //and store them in c[4] array 
void getPassword(){ 
for (int i=0 ; i<4 ; i++){ 
c[i]= myKeypad.waitForKey(); 
lcd.setCursor(i,1); 
lcd.print("*"); 
} 
lcd.clear(); 
  for (int j=0 ; j<4 ; j++){ //comparing the code entred with the codestored 
    if(c[j]==code[j]) ij++; //Everytime the char is correct weincrement the ij until it reaches 4 which means the code is correct 
  } //Otherwise it won't reach 4 andit will show "wrong code" as written above 
} 

//The Enrolling and getFingerprintEnroll functions are mainly basedonthe "Enroll" example code from the library //Only the modifications will be commented, return to the exampletosee each step in details, as here it's shortened 
void Enrolling() { 
keypressed = NULL; 
lcd.clear(); 
lcd.print("Enroll New");
delay(2000);
lcd.clear(); 
lcd.setCursor(0,0);
lcd.print("Enter new ID"); 
id = readnumber(); //This functiongets the Id it was meant to get it from the Serial monitor but we modified it 
if (id == 0) {// ID #0 not allowed, try again! 
return; 
} 
while (! getFingerprintEnroll() ); 
} 
//Enrolling function only the modifications are commented 
uint8_t getFingerprintEnroll() { 
int p = -1; 
lcd.clear(); 
lcd.print("Enroll ID:"); //Message to print for every step lcd.setCursor(10,0); 
lcd.print(id); 
lcd.setCursor(0,1); 
lcd.print("Place finger"); //First step 
while (p != FINGERPRINT_OK) { 
p = finger.getImage(); 
} 
// OK success! 
p = finger.image2Tz(1); 
switch (p) { 
case FINGERPRINT_OK: 
break; 
case FINGERPRINT_IMAGEMESS: 
return p; 
case FINGERPRINT_PACKETRECIEVEERR: 
return p; 
case FINGERPRINT_FEATUREFAIL: 
return p; 
case FINGERPRINT_INVALIDIMAGE: 
return p; 
default:
return p; 
} 
lcd.clear(); 
lcd.setCursor(0,0); 
lcd.print("Remove finger"); //After getting the first template successfully 
lcd.setCursor(0,1); 
lcd.print("please !"); 
delay(2000); 
p = 0; 
while (p != FINGERPRINT_NOFINGER) { 
p = finger.getImage(); 
} 
p = -1; 
lcd.clear(); 
lcd.setCursor(0,0); 
lcd.print("Place same"); //We launch the same thing another timetoget a second template of the same finger 
lcd.setCursor(0,1); 
lcd.print("finger please"); 
while (p != FINGERPRINT_OK) { 
p = finger.getImage(); 
} 
// OK success! 
p = finger.image2Tz(2); 
switch (p) { 
case FINGERPRINT_OK: 
break; 
case FINGERPRINT_IMAGEMESS: 
return p; 
case FINGERPRINT_PACKETRECIEVEERR: 
return p; 
case FINGERPRINT_FEATUREFAIL: 
return p; 
case FINGERPRINT_INVALIDIMAGE:
return p; 
default: 
return p; 
} 
p = finger.createModel(); 
if (p == FINGERPRINT_OK) { 
} else if (p == FINGERPRINT_PACKETRECIEVEERR) { 
return p; 
} else if (p == FINGERPRINT_ENROLLMISMATCH) { 
return p; 
} else { 
return p; 
} 
p = finger.storeModel(id); 
if (p == FINGERPRINT_OK) { 
lcd.clear(); //if both images are gotten without problem we store the template as the Id we entred lcd.setCursor(0,0); 
lcd.print("Stored in"); //Print a message after storing and showingthe ID where it's stored 
lcd.setCursor(0,1); 
lcd.print("ID: "); 
lcd.setCursor(5,1); 
lcd.print(id); 
delay(3000); 
lcd.clear(); 
} else if (p == FINGERPRINT_PACKETRECIEVEERR) { 
return p; 
} else if (p == FINGERPRINT_BADLOCATION) { 
return p; 
} else if (p == FINGERPRINT_FLASHERR) { 
return p; 
} else { 
return p; 
} 
}
//This function gets the ID number as 3 digits format like 001 for ID1//And return the number to the enrolling function 
uint8_t readnumber(void) { 
uint8_t num = 0; 
while (num == 0) { 
char keey = myKeypad.waitForKey(); 
int num1 = keey-48; 
lcd.setCursor(0,1); 
lcd.print(num1); 
keey = myKeypad.waitForKey(); 
int num2 = keey-48; 
lcd.setCursor(1,1); 
lcd.print(num2); 
keey = myKeypad.waitForKey(); 
int num3 = keey-48; 
lcd.setCursor(2,1); 
lcd.print(num3); 
delay(1000); 
num=(num1*100)+(num2*10)+num3; 
keey=NO_KEY; 
} 
return num; 
} 
//Main function taken from the "fingerprint" example and modified//Only the modifications are commented 
//This function waits for a fingerprint, scan it and give it access if recognised 
int getFingerprintIDez() { 
  uint8_t p = finger.getImage(); //Image scanning 
  if (p != FINGERPRINT_OK) return -1; 
  p = finger.image2Tz(); //Converting 
  if (p != FINGERPRINT_OK) return -1; 
  p = finger.fingerFastSearch(); //Looking for matches in theinternal memory 
  if (p != FINGERPRINT_OK){ //if the searching fails it meansthat the template isn't registered

    lcd.clear(); //And here we write a messageor take an action for the denied template 
    lcd.print("Access denied"); 
    delay(2000); 
    lcd.clear(); 
    lcd.print("Place finger"); 
    return -1; 
  } 
  //If we found a match we proceed in the function 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.write("COMPLETED! "); 
  lcd.setCursor(0, 1); 
  lcd.write("Found match. ID"); 
  lcd.setCursor(15, 1); 
  lcd.print(finger.fingerID, DEC); 
  delay(5000); 
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.write("INITIALISING..."); 
  lcd.setCursor(0, 1); 
  lcd.write("Password mode!"); 
  delay(3000); 
  ij=0; //ij is set to 0 lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("Enter Password"); 
  getPassword(); //Getting the passcode function 
  if(ij==4) 
  { //If the passcode is correct we can start enrolling newfinger template 
    digitalWrite (lockOutput, HIGH ); 
    lcd.clear(); 
    lcd.setCursor(0, 0); 
    lcd.write(" WELCOME "); 
    lcd.setCursor(0, 1); 
    lcd.write("Identity no. #");

    lcd.setCursor(12, 1); 
    lcd.print(finger.fingerID, DEC); 
    delay(5000); 
    digitalWrite (lockOutput, LOW ); 
  } 
  else{ //If the code is wrong we can't addnewusers (templates) 
    lcd.setCursor(0,0); 
    lcd.print("Wrong code"); 
    delay(2000); 
    lcd.clear(); 
    lcd.clear(); 
  } 
  lcd.print("Place finger"); 
  return finger.fingerID; 
}
