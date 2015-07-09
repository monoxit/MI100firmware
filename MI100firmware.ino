/*
  mi100firmware.ino

  Copyright (c) 2013-2014 Masami Yamakawa

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

Rev.7 2015     Add turn ON/OFF LED/Add move forward(R speed, L speed) G-R-R-R
Rev.6 20140526 Fix speed control/git initial G-R-R-G
Rev.5 20140524 Add startup motor test/Add speed control G-R-G-R
Rev.4 20140325 Fix blue LED micros overrun  R-G-B-B-G-R
Rev.3 20140121 Add photo sensor R-G-B-R-G-B
*/

#define LED_R  14 // LOW:ON,HIGH:OFF
#define LED_G  2
#define LED_B  6

#define PIEZO  7
#define VCAP   0
#define PHOTO  6

#define M_AIN1  10  // Right motor forward rotation (HIGH:ON/LOW:OFF)
#define M_AIN2  9   // Right motor backward rotation
#define M_PWMA  13  // Right motor speed (0 - 255)
#define M_BIN1  11  // Left motor backward rotation
#define M_BIN2  8   // Left motor forward rotation
#define M_PWMB  3   // Left motor speed
#define M_STBY  4

PROGMEM const byte kRecieveBufferSize = 35;
PROGMEM const byte kMaxCommandArgc = 4;
PROGMEM const int kFactorN = 3; //Simple LPF
PROGMEM const int kVRef = 3300; //mV
PROGMEM const int kAnalogMax = 255; //Anode common LEDs are tured off by writing this value.
PROGMEM const unsigned long kSpwmCycle = 25000; //micros
PROGMEM const unsigned short rfHeartbeatTimeout = 6000; //mS
PROGMEM const int kReadTimeout = 10000; //mS
PROGMEM const byte kPreFadeRatio = 1;
PROGMEM const byte kPostFadeRatio = 5;
PROGMEM const byte kFlatRatio = 10 - (kPreFadeRatio + kPostFadeRatio);
PROGMEM const int kMaxMoveDuration = 1000; //mS
PROGMEM const int kMaxTalkDuration = 3000; //mS
PROGMEM const int kMaxMotorSpeed = 255;

PROGMEM const int kAnalogUpdateRate = 50; //mS

short battery;
short light;

int motorSpeed = 1023;

unsigned long lastSerialRecieved = 0;
unsigned long lastAnalogUpdate = 0;
unsigned long spwmToggleTime=0, spwmStartTime=0, spwmCurrentTime;
byte spwmCycleState=1, spwmDuty, spwmPin;

byte lastRedDuty=50, lastGreenDuty=50, lastBlueDuty=50;
byte redValue=0, greenValue=0, blueValue=0;

void setup(){
  Serial.begin(9600);
  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_B, HIGH);
  pinMode(M_AIN1, OUTPUT);
  pinMode(M_AIN2, OUTPUT);
  pinMode(M_BIN1, OUTPUT);
  pinMode(M_BIN2, OUTPUT);
  pinMode(M_STBY, OUTPUT);
  digitalWrite(M_STBY, HIGH);

  battery = ((float)analogRead(VCAP) / 1024) * kVRef;
  light = analogRead(PHOTO);
  lastAnalogUpdate = millis();

  spwmPin = LED_B;

  tone(PIEZO, 220, 100);
  delay(500);
  tone(PIEZO, 880,100);
  delay(200);

  blinkRgbLed(0,30,0,0,100,0); // G:0
  delay(200);
  blinkRgbLed(30,0,0,0,100,0); // R:1
  delay(200);
  blinkRgbLed(30,0,0,0,100,0); // R:1
  delay(200);
  blinkRgbLed(30,0,0,0,100,0); // R:1

  //Motor test
  analogWrite(M_PWMA, 1023);
  analogWrite(M_PWMB, 1023);
  digitalWrite(M_AIN2, LOW);
  digitalWrite(M_BIN1, LOW);
  digitalWrite(M_AIN1, HIGH);
  digitalWrite(M_BIN2, HIGH);
  delay(10);
  stopMotors();
  delay(100);

  analogWrite(M_PWMA, 1023);
  analogWrite(M_PWMB, 1023);
  digitalWrite(M_AIN1, LOW);
  digitalWrite(M_BIN2, LOW);
  digitalWrite(M_AIN2, HIGH);
  digitalWrite(M_BIN1, HIGH);
  delay(10);
  stopMotors();
}


void loop(){
  short res;
  char buf[kRecieveBufferSize];
  char cmd;
  char *tempStr;
  unsigned int args[kMaxCommandArgc];
  byte i;
  int  duration;
  int  rightMotorSpeed, leftMotorSpeed;

  analogUpdate();

  if(battery < 1500){
    blinkRgbLed(20,0,0,0,100,0);
    delay(2000);
    return;
  }

  if(Serial.available() <= 0){
    if(millis() - lastSerialRecieved > rfHeartbeatTimeout){
      stopMotors();
      noTone(PIEZO);
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, LOW);
      delay(100);
      turnRgbLed(redValue,greenValue,blueValue);
      delay(900);
    }
    return;
  }

  if(!serialReadln(buf, sizeof(buf), kReadTimeout)) return;

  // Command Line Analysis
  cmd = buf[0];
  tempStr = strtok(buf, ",");
  for(tempStr = strtok(NULL, ","), i = 0; tempStr && i < sizeof(args); tempStr = strtok(NULL, ","), i++){
    args[i] = atoi(tempStr);
  }
  for(; i < sizeof(args); i++) args[i] = 0;
  res = battery;
  switch(cmd){
    case 'M':
      res = freeRam();
      break;

    case 'H':
      res = battery;
      break;

    case 'P':
      res = light;
      break;

    case 'L':
      // Left Pivot
      // L,duration(ms)
      // L,500
      duration = args[0] > kMaxMoveDuration ? kMaxMoveDuration : args[0];
      analogWrite(M_PWMA, motorSpeed);
      analogWrite(M_PWMB, motorSpeed);
      digitalWrite(M_AIN2, LOW);
      digitalWrite(M_BIN2, LOW);
      digitalWrite(M_AIN1, HIGH);
      digitalWrite(M_BIN1, HIGH);
      delay(duration);
      stopMotors();
      break;

     case 'R':
      // Right Pivot
      // R,duration(ms)
      // R,500
      duration = args[0] > kMaxMoveDuration ? kMaxMoveDuration : args[0];
      analogWrite(M_PWMA, motorSpeed);
      analogWrite(M_PWMB, motorSpeed);
      digitalWrite(M_AIN1, LOW);
      digitalWrite(M_BIN1, LOW);
      digitalWrite(M_AIN2, HIGH);
      digitalWrite(M_BIN2, HIGH);
      delay(duration);
      stopMotors();
      break;

    case 'A':
      // Turn Left
      // A,duration(ms)
      // A,500
      duration = args[0] > kMaxMoveDuration ? kMaxMoveDuration : args[0];
      analogWrite(M_PWMA, motorSpeed);
      analogWrite(M_PWMB, motorSpeed);
      digitalWrite(M_AIN2, LOW);
      digitalWrite(M_BIN2, LOW);
      digitalWrite(M_AIN1, HIGH);
      digitalWrite(M_BIN1, LOW);
      delay(duration);
      stopMotors();
      res = light;
      break;

    case 'U':
      // Turn Right
      // U,duration(ms)
      // U,500
      duration = args[0] > kMaxMoveDuration ? kMaxMoveDuration : args[0];
      analogWrite(M_PWMA, motorSpeed);
      analogWrite(M_PWMB, motorSpeed);
      digitalWrite(M_AIN1, LOW);
      digitalWrite(M_BIN1, LOW);
      digitalWrite(M_AIN2, LOW);
      digitalWrite(M_BIN2, HIGH);
      delay(duration);
      stopMotors();
      res = light;
      break;

    case 'F':
      // Move Forward
      // F,duration(ms)
      // F,500
      duration = args[0] > kMaxMoveDuration ? kMaxMoveDuration : args[0];
      analogWrite(M_PWMA, motorSpeed);
      analogWrite(M_PWMB, motorSpeed);
      digitalWrite(M_AIN2, LOW);
      digitalWrite(M_BIN1, LOW);
      digitalWrite(M_AIN1, HIGH);
      digitalWrite(M_BIN2, HIGH);
      delay(duration);
      stopMotors();
      break;

    case 'B':
      // Move Backward
      // B,duration(ms)
      // B,500
      duration = args[0] > kMaxMoveDuration ? kMaxMoveDuration : args[0];
      analogWrite(M_PWMA, motorSpeed);
      analogWrite(M_PWMB, motorSpeed);
      digitalWrite(M_AIN1, LOW);
      digitalWrite(M_BIN2, LOW);
      digitalWrite(M_AIN2, HIGH);
      digitalWrite(M_BIN1, HIGH);
      delay(duration);
      stopMotors();
      break;

    case 'S':
      // STOP mortors
      stopMotors();
      break;

    case 'D':
      // blink full color LED.
      // D,red_duty(%),green_duty(%),blue_duty(%),duration(ms)
      // D,100,50,70,500<CR|LF>
      lastRedDuty = args[0];
      lastGreenDuty = args[1];
      lastBlueDuty = args[2];
      duration = args[3] > kMaxTalkDuration ? kMaxTalkDuration : args[3];
      blinkRgbLed(lastRedDuty, lastGreenDuty, lastBlueDuty, (duration * kPreFadeRatio) / 10, (duration * kFlatRatio) / 10, (duration * kPostFadeRatio) / 10);
      break;

    case 'T':
      // Beep PIEZO
      // T,tone(Hz),duration(ms)
      // T,440,200<CR|LF>
      duration = args[1] > kMaxTalkDuration ? kMaxTalkDuration : args[1];
      noTone(PIEZO);
      tone(PIEZO, args[0], duration);
      break;

    case 'W':
      // W,PWM value
      // W,500<CR|LF>
      motorSpeed = args[0] > kMaxMotorSpeed ? kMaxMotorSpeed : args[0];
      break;

    case 'V':
      // turn on/off RGB LED.
      // V,red_value,green_value,blue_value (0;OFF, 0< value <=100;ON)
      // V,100,0,100<CR|LF>
      redValue = args[0] > 100 ? 100 : args[0];
      greenValue = args[1] > 100 ? 100 : args[1];
      blueValue = args[2] > 100 ? 100 : args[2];
      turnRgbLed(redValue, greenValue, blueValue);
      break;

    case 'Z':
      // Move Forward(rightMotorSpeed, leftMotorSpeed)
      // Z,rightMotorSpeed,leftMotorSpeed,duration(ms)
      // Z,100,200,30
      rightMotorSpeed = args[0] > kMaxMotorSpeed ? kMaxMotorSpeed : args[0];
      leftMotorSpeed = args[1] > kMaxMotorSpeed ? kMaxMotorSpeed : args[1];
      duration = args[2] > kMaxMoveDuration ? kMaxMoveDuration : args[2];
      analogWrite(M_PWMA, rightMotorSpeed);
      analogWrite(M_PWMB, leftMotorSpeed);
      digitalWrite(M_AIN2, LOW);
      digitalWrite(M_BIN1, LOW);
      digitalWrite(M_AIN1, HIGH);
      digitalWrite(M_BIN2, HIGH);
      delay(duration);
      stopMotors();
      motorSpeed = rightMotorSpeed < leftMotorSpeed ? rightMotorSpeed : leftMotorSpeed;
      res = light;
      break;
  }

  lastSerialRecieved = millis();
  Serial.print(cmd);
  Serial.print(',');
  Serial.println(res);
}


void stopMotors(){
  digitalWrite(M_AIN1, LOW);
  digitalWrite(M_AIN2, LOW);
  digitalWrite(M_BIN1, LOW);
  digitalWrite(M_BIN2, LOW);
}

boolean serialReadln(char *buf, int bufSize, int timeout){
  int i = 0;
  unsigned long expireTime = millis() + timeout;
  while(millis() < expireTime && i < bufSize){
    if(Serial.available() > 0){
      buf[i] = Serial.read();
      if(buf[i] == '\n' || buf[i] == '\r'){
        buf[i] = '\0';
        return true;
      }
      i++;
    }
  }
  return false;
}

void blinkRgbLed(byte redDuty, byte greenDuty, byte blueDuty, unsigned short preFadeTime, unsigned short flatTime, unsigned short postFadeTime){
  unsigned long startTime, endTime, currentTime, delta;
  byte reversedBlueDuty;
  unsigned short reversedRedDuty, reversedGreenDuty;
  float redPerMilliSec, greenPerMilliSec, bluePerMilliSec;

  noTone(PIEZO);

  if(redDuty > (byte)100)  redDuty = 100;
  if(greenDuty > (byte)100)  greenDuty = 100;
  if(blueDuty > (byte)100)  blueDuty = 100;

  reversedRedDuty = map(redDuty, 0, 100, kAnalogMax, 0);
  reversedGreenDuty = map(greenDuty, 0, 100, kAnalogMax, 0);
  reversedBlueDuty = 100 - blueDuty;

  redPerMilliSec = ((float)kAnalogMax - (float)reversedRedDuty) / (float)preFadeTime;
  greenPerMilliSec = ((float)kAnalogMax - (float)reversedGreenDuty) / (float)preFadeTime;
  bluePerMilliSec = (float)blueDuty / (float)preFadeTime;

  startTime = millis();
  spwmToggleTime = startTime;
  endTime = startTime + preFadeTime;
  while((currentTime = millis()) < endTime){
    delta = currentTime - startTime;
    analogWrite(LED_R, ((preFadeTime - delta) * redPerMilliSec) + reversedRedDuty);
    analogWrite(LED_G, ((preFadeTime - delta) * greenPerMilliSec) + reversedGreenDuty);
    spwmDuty = ((preFadeTime - delta) * bluePerMilliSec) + reversedBlueDuty;
    spwmUpdate();
  }

  startTime = millis();
  endTime = startTime + flatTime;
  analogWrite(LED_R, reversedRedDuty);
  analogWrite(LED_G, reversedGreenDuty);
  spwmDuty = reversedBlueDuty;
  while(millis() < endTime){
    spwmUpdate();
  }

  redPerMilliSec = ((float)kAnalogMax - (float)reversedRedDuty) / (float)postFadeTime;
  greenPerMilliSec = ((float)kAnalogMax - (float)reversedGreenDuty) / (float)postFadeTime;
  bluePerMilliSec = (float)blueDuty / (float)postFadeTime;

  startTime = millis();
  endTime = startTime + postFadeTime;
  while((currentTime = millis()) < endTime){
    delta = currentTime - startTime;
    analogWrite(LED_R, (delta * redPerMilliSec) + reversedRedDuty);
    analogWrite(LED_G, (delta * greenPerMilliSec) + reversedGreenDuty);
    spwmDuty = (delta * bluePerMilliSec) + reversedBlueDuty;
    spwmUpdate();
  }

  turnRgbLed(redValue,greenValue,blueValue);
}

void spwmUpdate(){
  spwmCurrentTime = micros();
  if(spwmCurrentTime >= spwmToggleTime){
    spwmStartTime = spwmCurrentTime;
    if(spwmCycleState == 1){
      digitalWrite(spwmPin, HIGH);
      spwmToggleTime = spwmStartTime + ((spwmDuty * kSpwmCycle) / 100);
      spwmCycleState = 0;
    }else{
      digitalWrite(spwmPin, LOW);
      spwmToggleTime = spwmStartTime + (((100-spwmDuty) * kSpwmCycle) / 100);
      spwmCycleState = 1;
    }
  }
}

void analogUpdate(){
  short vData, v;
  short lightData;

  if(millis() < lastAnalogUpdate + kAnalogUpdateRate) return;

  vData = analogRead(VCAP);
  v = ((float)vData / 1024) * kVRef;
  battery = (battery * (kFactorN - 1) + v) / kFactorN;

  lightData = analogRead(PHOTO);
  light = (light * (kFactorN - 1) + lightData) / kFactorN;
  lastAnalogUpdate = millis();
}

//
//Original Idea from https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
//
int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void turnRgbLed(byte redValue, byte greenValue, byte blueValue){
  byte digitalRedValue, digitalGreenValue, digitalBlueValue;

  digitalRedValue = redValue > 0 ? LOW : HIGH;
  digitalGreenValue = greenValue > 0 ? LOW : HIGH;
  digitalBlueValue = blueValue > 0 ? LOW : HIGH;

  digitalWrite(LED_R, digitalRedValue);
  digitalWrite(LED_G, digitalGreenValue);
  digitalWrite(LED_B, digitalBlueValue);
}
