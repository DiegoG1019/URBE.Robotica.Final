#define __ASSERT_USE_STDERR

#include <Arduino.h>
#include <Servo.h>
#include "Glass.h"

const int SpeedOfSound = 350; // m/s, at 30C
const int BaseSignalOut = 11;
const int HeadSignalOut = 10;
const int PumpSignalOut = 4;
const int USSignalOut = 7;
const int USEchoIn = 8;
const int OIRSignalIn = 9;

const int HeadIDLEPosition = 180;
const int HeadPreparedPosition = 90;

const float LEDFadeTimeDuringFill = 8; // 8 * 255 = 2040, just over 2 seconds

//                       Blue, Green, Red
const int StateLEDsOut[] = { 3, 5, 6 };

enum RobotState { Analyzing, OK, Error };
RobotState CurrentState = 0;

Servo Base;
Servo Head;

int CheckObstacle() {
  return digitalRead(OIRSignalIn) == LOW;
}

void MoveHeadToGlass(Glass* glass) {
  Base.write(glass->CenterDeg);
}

void FadeLEDTo(int pin, int mode, int millisecondsDelayPerFrame) {
  if (millisecondsDelayPerFrame == 0) {
    digitalWrite(pin, mode);
    return;
  }

  if (millisecondsDelayPerFrame < 1)
    millisecondsDelayPerFrame = 1;


  if (mode == LOW) {
    digitalWrite(pin, HIGH);
    for (int lvl = 255; lvl >= 0; lvl--) {
      analogWrite(pin, lvl);
      delay(millisecondsDelayPerFrame);
    }
    digitalWrite(pin, LOW);
  }
  else {
    digitalWrite(pin, LOW);
    for (int lvl = 0; lvl <= 255; lvl++) {
      analogWrite(pin, lvl);
      delay(millisecondsDelayPerFrame);
    }
    digitalWrite(pin, HIGH);
  }
}

void FadeLEDsTo(int mode, int millisecondsDelayPerFrame) {
  if (millisecondsDelayPerFrame == 0) {
    SetAllLEDsTo(mode);
    return;
  }

  if (millisecondsDelayPerFrame < 1)
    millisecondsDelayPerFrame = 1;

  if (mode == LOW) {
    SetAllLEDsTo(HIGH);
    for (int lvl = 255; lvl >= 0; lvl--) {
      for (int i = 0; i < sizeof(StateLEDsOut); i++)
        analogWrite(StateLEDsOut[i], lvl);
      delay(millisecondsDelayPerFrame);
    }
    SetAllLEDsTo(LOW);
  }
  else {
    SetAllLEDsTo(LOW);
    for (int lvl = 0; lvl <= 255; lvl++) {
      for (int i = 0; i < sizeof(StateLEDsOut); i++)
        analogWrite(StateLEDsOut[i], lvl);
      delay(millisecondsDelayPerFrame);
    }
    SetAllLEDsTo(HIGH);
  }
}

void SetAllLEDsTo(int mode) {
  for (int i = 0; i < sizeof(StateLEDsOut); i++)
    digitalWrite(StateLEDsOut[i], mode);
}

float ScanWaterLevel() {
  Serial.print(" [Scan] ");

  digitalWrite(USSignalOut, HIGH);
  delayMicroseconds(10);
  digitalWrite(USSignalOut, LOW);

  return pulseIn(USEchoIn, HIGH) / 59;
}

void PourWater() {
  Serial.print(" [Pour] ");
  digitalWrite(PumpSignalOut, HIGH);
  FadeLEDTo(StateLEDsOut[RobotState::OK], LOW, LEDFadeTimeDuringFill / 2);
  FadeLEDTo(StateLEDsOut[RobotState::OK], HIGH, LEDFadeTimeDuringFill / 2);
  digitalWrite(PumpSignalOut, LOW);


void SetState(RobotState state = 0) {
  if (state != CurrentState)
  {
    digitalWrite(StateLEDsOut[CurrentState], LOW);
    CurrentState = state;
    digitalWrite(StateLEDsOut[CurrentState], HIGH);
    Serial.print("Switched Robot state to '");
    switch (CurrentState)
    {
      case 0:
        Serial.print("Starting'"); // These have a ' at the end to match the opening one in "Switched Robot state to '". I'm aware it's ugly, but it works
        break;
      
      case 1:
        Serial.print("Analyzing'");
        break;
      
      case 2:
        Serial.print("OK'");
        break;

      case 3:
        Serial.print("Error'");
        break;

      default:
        Serial.print("Unknown'");
        break;
    }
  }
}

int main(void)
{
  init();

#if defined(USBCON)
  USBDevice.attach();
#endif
    
    
  for (int i = 0; i < sizeof(StateLEDsOut); i++)
    pinMode(StateLEDsOut[i], OUTPUT);

  FadeLEDsTo(HIGH, 10);

  Serial.begin (9600);
  Base.attach(BaseSignalOut);
  Head.attach(HeadSignalOut);
  pinMode(PumpSignalOut, OUTPUT);
  pinMode(USSignalOut, OUTPUT);
  pinMode(USEchoIn, INPUT);
  pinMode(OIRSignalIn, INPUT);

  digitalWrite(USSignalOut, LOW);
  digitalWrite(PumpSignalOut, LOW);
  Base.write(180);
  Head.write(HeadActivePosition);

  FadeLEDsTo(LOW, 2);
  FadeLEDsTo(HIGH, 2);
  FadeLEDsTo(LOW, 10);

  delay(1000);

  Base.write(0);
  Head.write(HeadIDLEPosition);

  delay(1000);

  for (;;) {
    Serial.println("Scanning for glasses");
    SetState(RobotState::Analyzing);
    int foundGlasses = Glass::DetectGlasses(Base, &CheckObstacle);
    if(foundGlasses > 0)
    {
      Glass* glass;
      SetState(RobotState::OK);
      Serial.print("Found ");
      Serial.print(foundGlasses);
      Serial.println(" glasses to fill");
      for(int i = 0; i < foundGlasses; i++){
        Serial.print("Acting on glass #");
        Serial.print(i);
        Glass::Get(i, glass);
        Serial.print(" [Get] ");
        MoveHeadToGlass(glass);
        Serial.print(" [Move] ");
        while (ScanWaterLevel() < 0.9)
          PourWater();
      }
    }

    if (serialEventRun) serialEventRun();
  }
        
    return 0;
}

