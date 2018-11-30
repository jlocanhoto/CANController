/*
#include "Error.h"

Error::Error(Error_Data &output)
{
    output.error_detected = LOW;
    this->output = &output;
    this->state = INIT__error_treatment__;
}

void Error::connect_inputs(Bit_Stuffing_Reading_Data &Bit_Stuff_Read, Decoder_Data &decoder, Frame_Transmitter_Data &Frame_Transmitter)
{
    this->input.Bit_Stuff_Read = &Bit_Stuff_Read;
    this->input.decoder = &decoder;
    this->input.Frame_Transmitter = &Frame_Transmitter;
}

void Error::run()
{
    switch(this->state)
    {
        case INIT__error_treatment__:
        {
            this->output->error_detected = LOW;
            if (this->input.Bit_Stuff_Read->stuff_error == HIGH || this->input.decoder->crc_error == HIGH || this->input.decoder->format_error == HIGH || this->input.Frame_Transmitter->bit_error == HIGH || this->input.Frame_Transmitter->ack_error == HIGH) {
                this->state = ERROR__error_treatment__;
            }
            break;
        }
        case ERROR__error_treatment__:
        {
            if (this->input.Bit_Stuff_Read->stuff_error == HIGH) {
            this->output->error_detected = HIGH;
            this->output->error_state = 
            this->state = INIT__error_treatment__;
            }
            else if (this->input.decoder->crc_error == HIGH) {
                this->output->error_detected = HIGH;
                this->output->error_state = 
                this->state = INIT__error_treatment__;
            }
            else if (this->input.decoder->format_error == HIGH) {
                this->output->error_detected = HIGH;
                this->output->error_state = 
                this->state = INIT__error_treatment__;
            }
            else if (this->input.Frame_Transmitter->bit_error == HIGH) {
                this->output->error_detected = HIGH;
                this->output->error_state = 
                this->state = INIT__error_treatment__;
            }
            else if (this->input.Frame_Transmitter->ack_error == HIGH) {
                this->output->error_detected = HIGH;
                this->output->error_state = 
                this->state = INIT__error_treatment__;
            }
            break;
        }

    }
}
*/