#include "TimerOne.h"
#include "BTL.h"
#include "config.h"

bool flag_TQ = false;
bool flag_finished_bit = false;

void TimeQuantum()
{
    digitalWrite(LED, digitalRead(LED) ^ 1);
    flag_TQ = true;
}

BitTimingLogic::BitTimingLogic()
{

}

void BitTimingLogic::setup(uint32_t _TQ, int8_t _T1, int8_t _T2, int8_t _SJW)
{
    this->limit_TSEG1 = _T1;
    this->limit_TSEG2 = _T2;
    this->sjw = _SJW;
    this->frequency_divider(_TQ);
}

void BitTimingLogic::run(bool input_bit, bool write_bit, bool &sampled_bit, bool &output_bit)
{
    sampled_bit = input_bit;
    output_bit = write_bit;

    if (flag_TQ) {
        flag_TQ = false;
    }
}

bool BitTimingLogic::update_simulation(bool reach_segment, uint8_t &j)
{
    bool ret = false;

    if (flag_TQ) {
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

void BitTimingLogic::edge_detector(bool scaled_clock, bool bus_idle)
{

}

void BitTimingLogic::bit_segmenter(bool &sample_point, bool &writing_point)
{

}