#include "TimerOne.h"
#include "BTL.h"
#include "config.h"

bool scaled_clock = false;
bool flag_finished_bit = false;

void TimeQuantum()
{
    digitalWrite(LED, digitalRead(LED) ^ 1);
    scaled_clock = true;
}

BitTimingLogic::BitTimingLogic()
{
    this->hardsync = false;
    this->resync = false;
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

    this->edge_detector(input_bit, scaled_clock, bus_idle);

    if (scaled_clock) {
        scaled_clock = false;
    }
}

bool BitTimingLogic::simulate(bool reach_segment, uint8_t &j)
{
    bool ret = false;

    if (scaled_clock) {
        if ((reach_segment) && (flag_finished_bit)) {
            flag_finished_bit = false;
            ret = true;
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

void BitTimingLogic::edge_detector(bool input_bit, bool scaled_clock, bool bus_idle)
{
    if (input_bit == DOMINANT) {
        if (bus_idle) {
            this->hardsync = true;
            this->resync = false;
        }
        else {
            this->hardsync = false;
            this->resync = true;
        }
    }
    else {
        this->hardsync = false;
        this->resync = false;
    }
}

void BitTimingLogic::bit_segmenter(bool &sample_point, bool &writing_point)
{

}