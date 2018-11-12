#include "TimerOne.h"
#include "BTL.h"
#include "config.h"

bool scaled_clock = LOW;
bool flag_finished_bit = false;
bool flag_sync = true;

void TimeQuantum()
{
    digitalWrite(TQ_CLK, HIGH);
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

void BitTimingLogic::run(bool &tq, bool input_bit, bool write_bit, bool &sampled_bit, bool &output_bit, bool &bus_idle, bool &sample_point, bool &writing_point)
{
    static bool prev_input_bit = RECESSIVE;

    sampled_bit = input_bit;
    output_bit = write_bit;    

    if (tq) {
        #if LOGGING
        Serial.println("<RUN>");
        #endif

        this->edge_detector(prev_input_bit, input_bit, bus_idle);
        this->bit_segmenter(sample_point, writing_point);

        if (flag_finished_bit) {
            prev_input_bit = input_bit;
        }
        
        tq = false;
        digitalWrite(TQ_CLK, LOW);
    }
}

bool BitTimingLogic::nextTQ(uint8_t pos, uint8_t &j, boolean &tq)
{
    bool ret = false;
    
    if (flag_finished_bit) {
        flag_finished_bit = false;
        flag_sync = false;
        ret = true;
        j = 0;
    }
    else if (scaled_clock) {
        tq = true;
        
        #if SIMULATION
        
        #if LOGGING
        Serial.println("<SIMULATE>");
        Serial.print("j = ");
        Serial.print(j, DEC);
        Serial.print(" ? pos = ");
        Serial.println(pos, DEC);
        #endif

        if (j < pos) {
            j++;
        }
        else if (j == pos) {
            flag_sync = true;
            j++;

            #if LOGGING
            Serial.println("flag_sync");
            #endif
        }

        #endif

        scaled_clock = LOW;
    }

    return ret;
}

void BitTimingLogic::frequency_divider(uint32_t _TQ)
{
    Timer1.initialize(_TQ);
    Timer1.attachInterrupt(TimeQuantum);
}

void BitTimingLogic::edge_detector(bool prev_input_bit, bool input_bit, bool &bus_idle)
{
    #if LOGGING
    Serial.print(prev_input_bit, DEC);
    Serial.print(" -> ");
    Serial.println(input_bit, DEC);
    #endif

    if (flag_sync) {
        if ((input_bit == DOMINANT) && (prev_input_bit == RECESSIVE)) {
            if (bus_idle) {
                this->hardsync = true;
                this->resync = false;
                
                #if SIMULATION
                bus_idle = false;
                #endif

                #if LOGGING
                Serial.println("hardsync");
                #endif
            }
            else {
                this->hardsync = false;
                this->resync = true;

                #if LOGGING
                Serial.println("resync");
                #endif
            }
        }

        flag_sync = false;
    }
    else {
        this->hardsync = false;
        this->resync = false;
    }
}

void initial_values_bit_segmenter(int8_t t1, int8_t t2, int8_t &p_count, bool &writing_point, bool &sample_point, uint8_t &state, int8_t &count_limit1, int8_t &count_limit2)
{
    #if LOGGING
    Serial.print(">> WRITING POINT (p_count = ");
    Serial.print(p_count, DEC);
    Serial.println(")");
    #endif

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
        digitalWrite(HARDSYNC, HIGH);

        digitalWrite(STATE_0, LOW);
        digitalWrite(STATE_1, LOW);
        
        #if LOGGING
        Serial.println(">> HARD_SYNC");
        #endif
        
        initial_values_bit_segmenter(this->limit_TSEG1, 0, p_count, writing_point, sample_point, state, count_limit1, count_limit2);
    }
    else {
        digitalWrite(HARDSYNC, LOW);
        digitalWrite(RESYNC, LOW);

        switch(state)
        {
            case SYNC_SEG:
                digitalWrite(STATE_0, LOW);
                digitalWrite(STATE_1, LOW);
        
                #if LOGGING
                Serial.println("state = SYNC_SEG");
                #endif

                initial_values_bit_segmenter(this->limit_TSEG1, 0, p_count, writing_point, sample_point, state, count_limit1, count_limit2);
                break;
            case TSEG1:
                digitalWrite(STATE_0, HIGH);
                digitalWrite(STATE_1, LOW);
        
                #if LOGGING
                Serial.println("state = TSEG1");
                #endif

                writing_point = LOW;
                
                if (p_count < count_limit1) {
                    p_count++;
                }

                if (this->resync) {
                    this->resync = false;
                    digitalWrite(RESYNC, HIGH);
                
                    #if LOGGING
                    Serial.print(">> RESYNC (p_count = ");
                    Serial.print(p_count, DEC);
                    Serial.println(")");
                    #endif

                    count_limit1 += min(this->sjw, p_count);
                }
                
                if (p_count == count_limit1) {
                    #if LOGGING
                    Serial.print(">> SAMPLE POINT (p_count = ");
                    Serial.print(p_count, DEC);
                    Serial.println(")");
                    #endif

                    sample_point = HIGH;
                    p_count = this->limit_TSEG2;
                    state = TSEG2;
                }

                break;
            case TSEG2:
                digitalWrite(STATE_0, LOW);
                digitalWrite(STATE_1, HIGH);
        
                #if LOGGING
                Serial.println("state = TSEG2");
                #endif

                sample_point = LOW;

                if (this->resync) {
                    this->resync = false;
                    digitalWrite(RESYNC, HIGH);
                
                    #if LOGGING
                    Serial.print(">> RESYNC (p_count = ");
                    Serial.print(p_count, DEC);
                    Serial.println(")");
                    #endif

                    phase_error = min(this->sjw, -p_count);
                    count_limit2 -= phase_error;
                }
                
                if (p_count < count_limit2) {
                    p_count++;
                }
                else if (p_count == count_limit2) {
                    #if LOGGING
                    Serial.print(phase_error, DEC);
                    Serial.print(" => ");
                    Serial.println(phase_error == -p_count, DEC);
                    #endif

                    if (phase_error == -p_count) {
                        #if LOGGING
                        Serial.println("it was SYNC_SEG");
                        #endif

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
    
    #if LOGGING
    Serial.print("p_count = ");
    Serial.println(p_count, DEC);
    #endif
}