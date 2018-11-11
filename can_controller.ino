#include "BTL.h"

void setup()
{
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    BitTimingLogic BTL(TQ, T1, T2, SJW);
}

void loop()
{

}