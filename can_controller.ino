#include "BTL.h"

// sequência de bits do frame de input
bool input[]      = {false, true, true, true, false, true};
// sequência de bits do frame de input
bool output[]     = {false, true, false, false, true, true};
// posição do bit segmentado em que ocorre o respectivo bit de input
uint8_t seg_pos[] = {  0  ,  0  ,  0  ,  0  ,   0  ,  0  };

BitTimingLogic BTL;

void setup()
{
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    BTL.setup(TQ, T1, T2, SJW);
}

void loop()
{
    static uint8_t i = 0, j = 0;
    
    //bool input_bit = digitalRead(...);
    bool input_bit = input[i];
    bool output_bit;
    bool sampled_bit;
    //bool write_bit = digitalRead(...);
    bool write_bit = output[i];

    BTL.run(input_bit, write_bit, sampled_bit, output_bit);
    
    if (i < 6) {
        Serial.print(sampled_bit, DEC);
        if (i < 5) {
            Serial.print(", ");
        }
        else {
            Serial.println();
        }
        i++;
    }
}