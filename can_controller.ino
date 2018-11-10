#include "TimerOne.h"

#define LED 13
#define TQ  1000000 // in microseconds

void callback()
{
    digitalWrite(LED, digitalRead(LED) ^ 1);
}

void setup()
{
    pinMode(LED, OUTPUT);
    Timer1.initialize(TQ);
    Timer1.attachInterrupt(callback);
}

void loop()
{

}