#include "BTL.h"

// sequência de bits do frame de input
bool input[]      = {HIGH, HIGH, LOW, HIGH, LOW, HIGH};
// sequência de bits do frame de input
bool output[]     = {LOW, HIGH, LOW, LOW, HIGH, HIGH};

/*****************************************************************************
| -------------------------------- BIT TIME -------------------------------- |
| SYNC_SEG |             TSEG1             |              TSEG2              |
|     0    | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
*****************************************************************************/
// posição do bit segmentado em que ocorre o respectivo bit de input
uint8_t seg_pos[] = {  0  ,  0  ,  14  ,  0  ,   0  ,  0  };

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
    static bool input_bit = input[i];
    //bool write_bit = digitalRead(...);
    static bool write_bit = output[i];
    static bool output_bit = LOW;
    static bool sampled_bit = LOW;
    static bool bus_idle = HIGH;
    static bool sample_point = LOW;
    static bool writing_point = LOW;
    bool simulated = false;
    
    if (i < 6) {
        if (BTL.simulate(seg_pos[i], j, simulated)) {            
            Serial.println();
            Serial.print(i, DEC);
            Serial.print(". sampled_bit = ");
            Serial.println(sampled_bit, DEC);
            Serial.println();

            i++;
            input_bit = input[i];
            write_bit = output[i];
        }

        BTL.run(simulated, input_bit, write_bit, sampled_bit, output_bit, bus_idle, sample_point, writing_point);
    }
}