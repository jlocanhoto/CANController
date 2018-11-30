#include "Bit_Stuffing_Reading.h"

Bit_Stuffing_Reading::Bit_Stuffing_Reading(Bit_Stuffing_Reading_Data &output)
{
    output.new_sampled_bit = RECESSIVE;
    output.new_sample_pt = LOW;

    this->count = 0;
    this->output = &output;
    this->state = INIT__Bit_Stuffing_Reading__;
    this->previous_bit = RECESSIVE;
    this->previous_sp_pt = LOW;
}

void Bit_Stuffing_Reading::connect_inputs(Decoder_Data &decoder, BTL_Data &BTL)
{
    this->input.BTL = &BTL;
    this->input.decoder = &decoder;
}

void Bit_Stuffing_Reading::run()
{
    bool sample_point_edge = false;
    
    if (this->previous_sp_pt == LOW && this->input.BTL->sample_point == HIGH) {
        sample_point_edge = true;
        this->output->new_sampled_bit = this->input.BTL->sampled_bit;
    }

    switch (this->state)
    {
        case INIT__Bit_Stuffing_Reading__:
        {
            this->count = 0;
            this->output->stuff_error = LOW;

            if (sample_point_edge) {
                this->count++;
                this->output->new_sample_pt = HIGH;
                this->state = STUFF__Bit_Stuffing_Reading__;
            }
            break;
        }
        case STUFF__Bit_Stuffing_Reading__:
        {
            this->output->new_sample_pt = LOW;

            if (this->input.decoder->stuffing_enable == LOW) {
                this->state = INIT__Bit_Stuffing_Reading__;
            }
            else if (sample_point_edge) {
                if(this->input.BTL->sampled_bit == this->previous_bit && this->count < 5) {
                    this->count++;
                    this->output->new_sample_pt = HIGH;
                }
                else if (this->input.BTL->sampled_bit != this->previous_bit && this->count < 5) {
                    this->count = 1;
                    this->output->new_sample_pt = HIGH;
                }
                else if (this->input.BTL->sampled_bit != this->previous_bit && this->count == 5) {
                    this->count = 0;
                }
                else if (this->input.BTL->sampled_bit == this->previous_bit && this->count == 5) {
                    this->state = ERROR__Bit_Stuffing_Reading__;
                    Serial.println("ERROR__Bit_Stuffing_Reading__");
                }

                this->previous_bit = this->input.BTL->sampled_bit;
            }
            break;
        }
        case ERROR__Bit_Stuffing_Reading__:
        {
            this->output->stuff_error = HIGH;
            this->state = INIT__Bit_Stuffing_Reading__;
        }
    }

    this->previous_sp_pt = this->input.BTL->sample_point;
}