#include <Keypad.h>

//evt emergency
bool emergencyShutdown = false;
bool openedforEmergency = false;
const int resetBtn = A2;
const int dangerLED = 10;

//evt keypad
const byte rows = 4; 
const byte columns = 4; 

byte rowPins[rows] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[columns] = {6, 7, 8, 9}; //connect to the column pinouts of the keypad

char myKeys[rows][columns] = { //4x4 matrix keypad display
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'0','#','*','D'}
};

Keypad myKeypad = Keypad(makeKeymap(myKeys), rowPins, colPins, rows, columns); //function

//evt motor
const int latchPin = 13;
const int clockPin = 12;
const int dataPin = 11; 

byte steps[] = {B00010001, B00010010, B00010100, B00011000}; // 4-step sequence pattern to 8 bit using the 74HC595

// pins for leak sensor
const int sensorVCC = A0;
const int sensorSig = A1;


void setup(){
  Serial.begin(9600);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  updateShiftRegister(0); // 74HC595 starts at 0 OFF
  pinMode(sensorVCC, OUTPUT);
  digitalWrite(sensorVCC, LOW);// off by default
  pinMode(resetBtn, INPUT_PULLUP); // button connects pinA2 to GND
  pinMode(dangerLED, OUTPUT);
  digitalWrite(dangerLED, LOW);//red LED off
  updateShiftRegister(0);
}

void updateShiftRegister(byte data){ // for 74HC595 decoder, LOW -> take in -> HIGH
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}


void loop(){
  //below evt for emergency shutdown
  if (digitalRead(resetBtn) == LOW){ // -> if button is pressed
    if (emergencyShutdown){ // -> if shutdown is true -> there is a shutdown
      //shutdown happening + button pressed = no more shutdown
      Serial.println(F("System Reset - Emergency Cleared"));
      if (openedforEmergency){
        for (int i = 0; i < 128; i++){ // shift it back
            updateShiftRegister(steps[3]); delay(3);
            updateShiftRegister(steps[2]); delay(3);
            updateShiftRegister(steps[1]); delay(3);
            updateShiftRegister(steps[0]); delay(3);
          }
      }
      emergencyShutdown = false;
      openedforEmergency = false;
      digitalWrite(dangerLED, LOW);
      updateShiftRegister(0);
      delay(100);
      return;
      }
      updateShiftRegister(0);
    } 

  if (emergencyShutdown){
    digitalWrite(dangerLED, HIGH);
    if (!openedforEmergency){ // if door is not opened
      Serial.println(F("Opening door for emergency"));
      for (int i = 0; i < 128; i++){
          for (int s = 0; s < 4; s++){
            updateShiftRegister(steps[s]);
            delay(5);
          }
        }
      }
    openedforEmergency = true; // leave door open
    updateShiftRegister(B00010000); // leave only LED open and door doesnt move
  }

  else {
    //below evt for key
    char myKey = myKeypad.getKey();
    if (myKey){
      Serial.println(myKey);
      if (myKey == 'A'){
        Serial.println(F("CORRECT INPUT - LED ON"));
      

        //below evt for door
        for (int i = 0; i<128; i++){
          for (int s = 0; s<4; s++){
            updateShiftRegister(steps[s]);
            delay(5);
          }
        }
        updateShiftRegister(B00010000); // motor off, only LED on
        delay(3000);

      
        for (int i = 0; i < 128; i++){ // shift it back
          updateShiftRegister(steps[3]); delay(3);
          updateShiftRegister(steps[2]); delay(3);
          updateShiftRegister(steps[1]); delay(3);
          updateShiftRegister(steps[0]); delay(3);
        }

        updateShiftRegister(0);
      }
      else {
        Serial.println(F("INCORRECT INPUT - LED OFF"));
        updateShiftRegister(0);
      }
    }

    //below evt for leak
    digitalWrite(sensorVCC, HIGH);
    delay(100);
    int leakValue = analogRead(sensorSig);
    digitalWrite(sensorVCC, LOW);

      if (leakValue < 700 && leakValue >5) { // <500 very wet, >900 is dry
        Serial.println(F("Leak Detected!"));
        Serial.println(leakValue);
        emergencyShutdown = true;
      }
    }
  }