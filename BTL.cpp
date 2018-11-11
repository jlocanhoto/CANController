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
        this->bit_segmenter(sample_point, writing_point);
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
        /*
        Serial.print("Hardsync = ");
        Serial.println(this->hardsync, DEC);
        Serial.print("Resync = ");
        Serial.println(this->resync, DEC);
        */
    }
}

void initial_values_bit_segmenter(int8_t t1, int8_t t2, int8_t &p_count, bool &writing_point, bool &sample_point, uint8_t &state, int8_t &count_limit1, int8_t &count_limit2)
{
    p_count = 0;
    writing_point = HIGH;
    sample_point = LOW;
    state = TSEG1;
    count_limit1 = t1;
    count_limit2 = t2;
}

void BitTimingLogic::bit_segmenter(bool &sample_point, bool &writing_point)
{
    static uint8_t state = SYNC_SEG;
    static int8_t p_count = 0;
    static int8_t count_limit1 = 0, count_limit2 = 0;
    int8_t phase_error;

    if (this->hardsync) {
        Serial.println(">> HARD_SYNC");
        initial_values_bit_segmenter(this->limit_TSEG1, 0, p_count, writing_point, sample_point, state, count_limit1, count_limit2);
    }
    else {
        switch(state)
        {
            case SYNC_SEG:
                Serial.println("state = SYNC_SEG");
                initial_values_bit_segmenter(this->limit_TSEG1, 0, p_count, writing_point, sample_point, state, count_limit1, count_limit2);
                break;
            case TSEG1:
                Serial.println("state = TSEG1");
                writing_point = LOW;                

                if (this->resync) {
                    Serial.println(">> RESYNC");
                    count_limit1 += min(this->sjw, p_count);
                }
                
                if (p_count < count_limit1) {
                    p_count++;
                }
                else { // p_count == count_limit1
                    sample_point = HIGH;
                    p_count = this->limit_TSEG2;
                    state = TSEG2;
                }

                break;
            case TSEG2:
                Serial.println("state = TSEG2");
                sample_point = LOW;                

                if (this->resync) {
                    Serial.println(">> RESYNC");
                    phase_error = min(this->sjw, -p_count);
                    count_limit2 -= phase_error;
                }
                
                if (p_count < count_limit2) {
                    p_count++;
                }
                else { // p_count == count_limit2
                    if (phase_error == -p_count) {
                        Serial.println("it was SYNC_SEG");
                        initial_values_bit_segmenter(this->limit_TSEG1, 0, p_count, writing_point, sample_point, state, count_limit1, count_limit2);
                    }
                    else {
                        state = SYNC_SEG;   
                    }
                }

                break;
        }
    }
}