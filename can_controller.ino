#include "config.h"
#include "TimerOne.h"

void TimeQuantum()
{
    digitalWrite(LED, digitalRead(LED) ^ 1);
}

void setup()
{
    pinMode(LED, OUTPUT);
    Timer1.initialize(TQ);
    Timer1.attachInterrupt(TimeQuantum);
}

void loop()
{

}