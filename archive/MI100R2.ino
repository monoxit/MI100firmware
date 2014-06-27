/*
  mi100r2.ino

  Copyright (c) 2013-2014 Masami Yamakawa

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  Rev.2 Initial release  R-G-B
*/

#define LED_R  14
#define LED_G  2
#define LED_B  6

#define PIEZO  7
#define VCAP   0

#define M_AIN1  10
#define M_AIN2  9
#define M_PWMA  13
#define M_BIN1  11
#define M_BIN2  8
#define M_PWMB  3
#define M_STBY  4

PROGMEM const byte recieveBufferSize = 35;
PROGMEM const byte maxCommandArgc = 4;
PROGMEM const int factorN = 3; //Simple LPF for voltage sensor
PROGMEM const int vRef = 3300; //mV
PROGMEM const int analogMax = 260; //Anode common LEDs are tured off by writing this value.
PROGMEM const unsigned long spwmCycle = 25000; //micros
PROGMEM const unsigned short rfHeartbeatTimeout = 3000;
PROGMEM const int readTimeout = 10000;
PROGMEM const byte preFadeRatio = 1;
PROGMEM const byte postFadeRatio = 5;
PROGMEM const byte flatRatio = 10 - (preFadeRatio + postFadeRatio);
PROGMEM const int maxMoveDuration = 1000;
PROGMEM const int maxTalkDuration = 3000;

short vold, vnew;
unsigned long lastSerialRecieved=0;
unsigned long spwmToggleTime=0, spwmStartTime=0, spwmCurrentTime;
byte spwmCycleState=1, spwmDuty, spwmPin;

byte lastRedDuty=50, lastGreenDuty=50, lastBlueDuty=50;

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
  
  vold = ((float)analogRead(VCAP) / 1024) * vRef;
  spwmPin = LED_B;
  
  tone(PIEZO, 220, 100);
  delay(500);
  tone(PIEZO, 880,100);
  delay(200);
  
  brinkRgbLed(30,0,0,10,200,50);
  brinkRgbLed(0,20,0,10,200,50);
  brinkRgbLed(0,0,20,10,200,50);
  
}

void loop(){
  
  short va, v;
  char buf[recieveBufferSize];
  char cmd;
  char *tempStr;
  unsigned int args[maxCommandArgc];
  byte i;
  int  duration;
  
  va = analogRead(VCAP);
  v = ((float)va / 1024) * vRef;
  vnew = (vold * (factorN - 1) + v) / factorN;
  vold = vnew;
  
  //Serial.println(vnew);
  
  if(vnew < 1500){
    brinkRgbLed(20,0,0,0,100,0);
    delay(2000);
    return;
  }

  if(Serial.available() <= 0){
    if(millis() - lastSerialRecieved > rfHeartbeatTimeout){
      stopMotors();
      noTone(PIEZO);
      brinkRgbLed(0,0,50,0,50,0);
      delay(900);
    }
    return;
  }
  
  if(!serialReadln(buf, sizeof(buf), readTimeout)) return;
  
  // Command Line Analysis
  cmd = buf[0];
  tempStr = strtok(buf, ",");
  for(tempStr = strtok(NULL, ","), i = 0; tempStr && i < sizeof(args); tempStr = strtok(NULL, ","), i++){
    args[i] = atoi(tempStr);
  }
  for(; i < sizeof(args); i++) args[i] = 0;
    
  switch(cmd){
    case 'H':
      break;
      
    case 'L':
      // Left Pivot
      // L,duration(ms)
      // L,500
      duration = args[0] > maxMoveDuration ? maxMoveDuration : args[0];
      analogWrite(M_PWMA, 1023);
      analogWrite(M_PWMB, 1023);
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
      duration = args[0] > maxMoveDuration ? maxMoveDuration : args[0];
      analogWrite(M_PWMA, 1023);
      analogWrite(M_PWMB, 1023);
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
      duration = args[0] > maxMoveDuration ? maxMoveDuration : args[0];
      analogWrite(M_PWMA, 1023);
      analogWrite(M_PWMB, 1023);
      digitalWrite(M_AIN2, LOW);
      digitalWrite(M_BIN2, LOW);
      digitalWrite(M_AIN1, HIGH);
      digitalWrite(M_BIN1, LOW);
      delay(duration);
      stopMotors();
      break;  
    
    case 'U':
      // Turn Right
      // U,duration(ms)
      // U,500
      duration = args[0] > maxMoveDuration ? maxMoveDuration : args[0];
      analogWrite(M_PWMA, 1023);
      analogWrite(M_PWMB, 1023);
      digitalWrite(M_AIN1, LOW);
      digitalWrite(M_BIN1, LOW);
      digitalWrite(M_AIN2, LOW);
      digitalWrite(M_BIN2, HIGH);
      delay(duration);
      stopMotors();
      break; 
      
    case 'F':
      // Move Forward
      // F,duration(ms)
      // F,500
      duration = args[0] > maxMoveDuration ? maxMoveDuration : args[0];
      analogWrite(M_PWMA, 1023);
      analogWrite(M_PWMB, 1023);
      digitalWrite(M_AIN2, LOW);
      digitalWrite(M_BIN1, LOW);     
      digitalWrite(M_AIN1, HIGH);
      digitalWrite(M_BIN2, HIGH);
      delay(duration);
      stopMotors();
      break;
      
    case 'B':
      duration = args[0] > maxMoveDuration ? maxMoveDuration : args[0];
      analogWrite(M_PWMA, 1023);
      analogWrite(M_PWMB, 1023);
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
      // Brink full color LED.
      // D,red_duty(%),green_duty(%),blue_duty(%),duration(ms)
      // D,100,50,70,500<CR|LF>
      lastRedDuty = args[0];
      lastGreenDuty = args[1];
      lastBlueDuty = args[2];
      duration = args[3] > maxTalkDuration ? maxTalkDuration : args[3];
      brinkRgbLed(lastRedDuty, lastGreenDuty, lastBlueDuty, (duration * preFadeRatio) / 10, (duration * flatRatio) / 10, (duration * postFadeRatio) / 10);
      break;
    
    case 'T':
      // Beep PIEZO
      // T,tone(Hz),duration(ms)
      // T,440,200<CR|LF>
      duration = args[1] > maxTalkDuration ? maxTalkDuration : args[1];
      noTone(PIEZO);
      tone(PIEZO, args[0], duration);
      break;
  }
  //if(cmd != 'D' && cmd != 'T') brinkRgbLed(lastRedDuty,lastGreenDuty,lastBlueDuty,0,100,50);
  lastSerialRecieved = millis();
  Serial.print(cmd);
  Serial.print(',');
  Serial.println(vnew);
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

void brinkRgbLed(byte redDuty, byte greenDuty, byte blueDuty, unsigned short preFadeTime, unsigned short flatTime, unsigned short postFadeTime){
  unsigned long startTime, endTime, currentTime, delta;
  byte reversedBlueDuty;
  unsigned short reversedRedDuty, reversedGreenDuty;
  float redPerMilliSec, greenPerMilliSec, bluePerMilliSec;
  
  noTone(PIEZO);

  if(redDuty > (byte)100)  redDuty = 100;
  if(greenDuty > (byte)100)  greenDuty = 100;
  if(blueDuty > (byte)100)  blueDuty = 100;
  
  reversedRedDuty = (analogMax * (100 - (unsigned long)redDuty)) / 100;
  reversedGreenDuty = (analogMax * (100 - (unsigned long)greenDuty)) / 100;
  reversedBlueDuty = 100 - blueDuty;

  redPerMilliSec = ((float)analogMax - (float)reversedRedDuty) / (float)preFadeTime;
  greenPerMilliSec = ((float)analogMax - (float)reversedGreenDuty) / (float)preFadeTime;
  bluePerMilliSec = (float)blueDuty / (float)preFadeTime;
 
  startTime = millis();
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

  redPerMilliSec = ((float)analogMax - (float)reversedRedDuty) / (float)postFadeTime;
  greenPerMilliSec = ((float)analogMax - (float)reversedGreenDuty) / (float)postFadeTime;
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
  
  digitalWrite(LED_R,HIGH);
  digitalWrite(LED_G,HIGH);
  digitalWrite(LED_B,HIGH);
}

void spwmUpdate(){
  spwmCurrentTime = micros();
  if(spwmCurrentTime >= spwmToggleTime){
    spwmStartTime = spwmCurrentTime;
    if(spwmCycleState == 1){
      digitalWrite(spwmPin, HIGH);
      spwmToggleTime = spwmStartTime + ((spwmDuty * spwmCycle) / 100);
      spwmCycleState = 0;
    }else{
      digitalWrite(spwmPin, LOW);
      spwmToggleTime = spwmStartTime + (((100-spwmDuty) * spwmCycle) / 100);
      spwmCycleState = 1;
    }
  }
}
