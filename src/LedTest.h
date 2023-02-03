#pragma once

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

//***************************//
#define LEDR 24
#define LEDY 10
#define LEDG 9
//***************************//
#define VALUE_MAX 30

#define DELAY_TIME 1000000

int GPIOExport(int);
int GPIOUnexport(int);
int GPIODirection(int, int);
int GPIOWrite(int, int);
void RedBlink(int);
void YellowBlink(int);
void GreenBlink(int);
void LedTest();
void Exiting(int);