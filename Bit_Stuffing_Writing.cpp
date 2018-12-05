#include "Bit_Stuffing_Writing.h"

Bit_Stuffing_Writing::Bit_Stuffing_Writing(Bit_Stuffing_Writing_Data &output)
{
    this->count = 0;
    output.output_bit = RECESSIVE;
    output.arb_wr_pt = LOW;
    this->output = &output;
    this->state = INIT__Bit_Stuffing_Writing__;
    this->previous_bit = RECESSIVE;
    this->previous_wr_pt = LOW;
}

void Bit_Stuffing_Writing::connect_inputs(Frame_Transmitter_Data &frame_transmitter, BTL_Data &BTL)
{
    this->input.BTL = &BTL;
    this->input.frame_transmitter = &frame_transmitter;
}

void Bit_Stuffing_Writing::run()
{
    bool writing_point_edge = false;

    if (this->previous_wr_pt == LOW && this->input.BTL->writing_point == HIGH) {
        writing_point_edge = true;
        //Serial.println("BIT_STUFF->writing_point_edge");
    }

    this->output->output_bit = this->input.frame_transmitter->arb_output;

    switch (this->state)
    {
        case INIT__Bit_Stuffing_Writing__:
        {
            //Serial.println("INIT__Bit_Stuffing_Writing__");
            this->count = 0;

            if (writing_point_edge) {
                this->count++;
                this->output->arb_wr_pt = HIGH;
                this->state = STUFF__Bit_Stuffing_Writing__;
            }            
            break;
        }
        case STUFF__Bit_Stuffing_Writing__:
        {
            //Serial.println("STUFF__Bit_Stuffing_Writing__");
            this->output->arb_wr_pt = LOW;

            if (this->input.frame_transmitter->stuffing_enable == LOW) {
                this->state = INIT__Bit_Stuffing_Writing__;
            }
            else if (writing_point_edge) {
                if (this->input.frame_transmitter->arb_output == this->previous_bit && this->count < 5) {
                    this->count++;
                    this->output->arb_wr_pt = HIGH;
                }
                else if (this->input.frame_transmitter->arb_output != this->previous_bit && this->count <= 5) {
                    if (this->count < 5) {
                        this->output->arb_wr_pt = HIGH;
                    }

                    this->count = 1;                    
                }
                else if (this->input.frame_transmitter->arb_output == this->previous_bit && this->count == 5) {
                    this->count = 0;
                    this->output->output_bit = !this->input.frame_transmitter->arb_output;
                }
            }
            break;
        }
    }

    if (writing_point_edge) {
        this->previous_bit = this->input.frame_transmitter->arb_output;
    }

    this->previous_wr_pt = this->input.BTL->writing_point;
}