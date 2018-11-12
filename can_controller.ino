#include "BTL.h"

// sequência de bits do frame de input
bool input[]      = {LOW , HIGH, LOW , HIGH, LOW , HIGH, LOW };
// sequência de bits do frame de input
bool output[]     = {LOW , HIGH, LOW , LOW , HIGH, HIGH, LOW };

// posição do bit segmentado em que ocorre o respectivo bit de input
uint8_t seg_pos[] = {  0 ,  0  ,  15 ,  0  ,  8  ,  0  ,  1  };
/*****************************************************************************
| -------------------------------- BIT TIME -------------------------------- |
| SYNC_SEG |             TSEG1             |              TSEG2              |
|     0    | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|============================================================================|
|     0    | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |-7 | -6 | -5 | -4 | -3 | -2 | -1 |
*****************************************************************************/

BitTimingLogic BTL;

void setup()
{
    pinMode(TQ_CLK,   OUTPUT);
    pinMode(HARDSYNC, OUTPUT);
    pinMode(RESYNC,   OUTPUT);
    pinMode(STATE_0,  OUTPUT);
    pinMode(STATE_1,  OUTPUT);

    digitalWrite(TQ_CLK,   LOW);
    digitalWrite(HARDSYNC, LOW);
    digitalWrite(RESYNC,   LOW);
    digitalWrite(STATE_0,  LOW);
    digitalWrite(STATE_1,  LOW);

    Serial.begin(115200);
    BTL.setup(TQ, T1, T2, SJW);
}

void loop()
{
    static uint8_t i = 0, j = 0;
    
    #if SIMULATION
    static bool input_bit = input[i];
    static bool write_bit = output[i];
    #else
    bool input_bit = digitalRead(INPUT_BIT);
    bool write_bit = digitalRead(WRITE_BIT);
    #endif

    static bool output_bit = LOW;
    static bool sampled_bit = LOW;
    static bool bus_idle = HIGH;
    static bool sample_point = LOW;
    static bool writing_point = LOW;
    bool tq = false;
    
    if (i < sizeof(seg_pos)) {
        if (BTL.nextTQ(seg_pos[i], j, tq)) {
            #if LOGGING
            Serial.println();
            Serial.print(i, DEC);
            Serial.print(". sampled_bit = ");
            Serial.println(sampled_bit, DEC);
            Serial.println();
            #endif

            i++;
            input_bit = input[i];
            write_bit = output[i];
        }

        BTL.run(tq, input_bit, write_bit, sampled_bit, output_bit, bus_idle, sample_point, writing_point);
    }
    else {
        Serial.println("FINISHED");
    }
}