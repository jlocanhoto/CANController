#include "TimerOne.h"
#include "BTL.h"
#include "config.h"

bool scaled_clock = LOW;
bool flag_finished_bit = false;
bool flag_sync = true;

void TimeQuantum()
{
    digitalWrite(LED, digitalRead(LED) ^ 1);
    scaled_clock = HIGH;
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

void BitTimingLogic::run(bool simulated, bool input_bit, bool write_bit, bool &sampled_bit, bool &output_bit, bool &bus_idle, bool &sample_point, bool &writing_point)
{
    sampled_bit = input_bit;
    output_bit = write_bit;    

    if (simulated) {
        Serial.println("<RUN>");
        this->edge_detector(input_bit, bus_idle);
        this->bit_segmenter(sample_point, writing_point);
        
        simulated = false;
    }
}

bool BitTimingLogic::simulate(uint8_t pos, uint8_t &j, boolean &simulated)
{
    bool ret = false;

    if (scaled_clock) {
        simulated = true;
        Serial.println("<SIMULATE>");

        if (flag_finished_bit) {
            flag_finished_bit = false;
            flag_sync = false;
            ret = true;
            j = 0;
        }
        
        if (j < pos) {
            j++;
        }
        else if (j == pos) {
            flag_sync = true;
            j++;
        }

        scaled_clock = LOW;
    }

    return ret;
}

void BitTimingLogic::frequency_divider(uint32_t _TQ)
{
    Timer1.initialize(_TQ);
    Timer1.attachInterrupt(TimeQuantum);
}

void BitTimingLogic::edge_detector(bool input_bit, bool &bus_idle)
{
    static bool prev_input_bit = RECESSIVE;

    if (flag_sync) {
        if ((input_bit == DOMINANT) && (prev_input_bit == RECESSIVE)) {
            if (bus_idle) {
                this->hardsync = true;
                this->resync = false;
                bus_idle = false;
            }
            else {
                this->hardsync = false;
                this->resync = true;
            }
        }

        flag_sync = false;
        prev_input_bit = input_bit;
    }
    else {
        this->hardsync = false;
        this->resync = false;
    }
}

void initial_values_bit_segmenter(int8_t t1, int8_t t2, int8_t &p_count, bool &writing_point, bool &sample_point, uint8_t &state, int8_t &count_limit1, int8_t &count_limit2)
{
    Serial.println(">> WRITING POINT");
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
    int8_t phase_error = 127;
    bool transition = false;

    if (this->hardsync) {
        this->hardsync = false;
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
                    Serial.print(">> RESYNC (");
                    Serial.print(p_count, DEC);
                    Serial.println(")");
                    count_limit1 += min(this->sjw, p_count);
                }
                
                if (p_count < count_limit1) {
                    p_count++;
                }
                
                if (p_count == count_limit1) {
                    Serial.println(">> SAMPLE POINT");
                    sample_point = HIGH;
                    p_count = this->limit_TSEG2;
                    state = TSEG2;
                }

                break;
            case TSEG2:
                Serial.println("state = TSEG2");
                sample_point = LOW;

                if (this->resync) {
                    Serial.print(">> RESYNC (");
                    Serial.print(p_count, DEC);
                    Serial.println(")");
                    phase_error = min(this->sjw, -p_count);
                    count_limit2 -= phase_error;
                }
                
                if (p_count < count_limit2) {
                    p_count++;
                }

                if (p_count == count_limit2) {
                    Serial.print(phase_error, DEC);
                    Serial.print(" => ");
                    Serial.println(phase_error == -p_count, DEC);
                    if (phase_error == -p_count) {
                        Serial.println("it was SYNC_SEG");
                        initial_values_bit_segmenter(this->limit_TSEG1, 0, p_count, writing_point, sample_point, state, count_limit1, count_limit2);
                    }
                    else {
                        state = SYNC_SEG;
                    }

                    flag_finished_bit = true;
                }

                break;
        }
    }

    Serial.print("p_count = ");
    Serial.println(p_count, DEC);
}