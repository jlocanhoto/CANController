#include "TimerOne.h"
#include "BTL.h"
#include "config.h"

bool scaled_clock = LOW;
bool flag_finished_bit = HIGH;

void TimeQuantum()
{
    digitalWrite(LED, digitalRead(LED) ^ 1);
    scaled_clock = HIGH;
}

BitTimingLogic::BitTimingLogic()
{
    this->hardsync = LOW;
    this->resync = LOW;
}

void BitTimingLogic::setup(uint32_t _TQ, int8_t _T1, int8_t _T2, int8_t _SJW)
{
    this->limit_TSEG1 = _T1;
    this->limit_TSEG2 = _T2;
    this->sjw = _SJW;
    this->frequency_divider(_TQ);
}

void BitTimingLogic::run(bool input_bit, bool write_bit, bool &sampled_bit, bool &output_bit, bool bus_idle, bool &sample_point, bool &writing_point)
{
    sampled_bit = input_bit;
    output_bit = write_bit;

    this->edge_detector(input_bit, bus_idle);

    if (scaled_clock) {
        scaled_clock = LOW;
    }
}

bool BitTimingLogic::simulate(bool reach_segment, uint8_t &j)
{
    bool ret = LOW;

    if (scaled_clock) {
        if ((reach_segment) && (flag_finished_bit)) {
            //flag_finished_bit = LOW;
            ret = HIGH;
            j = 0;
        }
        else {
            j++;
        }
    }

    return ret;
}

void BitTimingLogic::frequency_divider(uint32_t _TQ)
{
    Timer1.initialize(_TQ);
    Timer1.attachInterrupt(TimeQuantum);
}

void BitTimingLogic::edge_detector(bool input_bit, bool bus_idle)
{
    static bool prev_input_bit = RECESSIVE;

    if (scaled_clock) {
        if ((input_bit == DOMINANT) && (prev_input_bit == RECESSIVE)) {
            if (bus_idle) {
                this->hardsync = HIGH;
                this->resync = LOW;
            }
            else {
                this->hardsync = LOW;
                this->resync = HIGH;
            }
        }
        else {
            this->hardsync = LOW;
            this->resync = LOW;
        }

        prev_input_bit = input_bit;

        Serial.print("Hardsync = ");
        Serial.println(this->hardsync, DEC);
        Serial.print("Resync = ");
        Serial.println(this->resync, DEC);
    }
}

void BitTimingLogic::bit_segmenter(bool &sample_point, bool &writing_point)
{

}