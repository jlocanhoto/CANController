#include "BTL.h"

// sequência de bits do frame de input
bool input[]      = {LOW, HIGH, LOW, HIGH, LOW, HIGH};
// sequência de bits do frame de input
bool output[]     = {LOW, HIGH, LOW, LOW, HIGH, HIGH};
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
    uint8_t old_i = i;
    //bool input_bit = digitalRead(...);
    bool input_bit = input[i];
    //bool write_bit = digitalRead(...);
    bool write_bit = output[i];
    static bool output_bit = LOW;
    static bool sampled_bit = LOW;
    static bool bus_idle = HIGH;
    static bool sample_point = LOW;
    static bool writing_point = LOW;
    
    if (BTL.simulate((j == seg_pos[i]), j)) {
        if (i < 6) {
            if (i == 1) {
                bus_idle = LOW;
            }

            i++;
        }
    }

    if (i != old_i) {
        BTL.run(input_bit, write_bit, sampled_bit, output_bit, bus_idle, sample_point, writing_point);

        Serial.print("sampled_bit = ");
        Serial.println(sampled_bit, DEC);
        Serial.println();
    }
}