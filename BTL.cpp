#include "TimerOne.h"
#include "BTL.h"
#include "config.h"

void TimeQuantum()
{
    digitalWrite(LED, digitalRead(LED) ^ 1);
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