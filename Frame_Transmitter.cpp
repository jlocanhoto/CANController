#include "Frame_Transmitter.h"
#include "Error.h"

Frame_Transmitter::Frame_Transmitter(Frame_Transmitter_Output &output)
{
    output.lost_arbitration = LOW;
    this->output = &output;
    this->state = INIT__Frame_Transmitter__;
}

void Frame_Transmitter::setup(Frame_Mounter_Output &frame_mounter, Bit_Stuffing_Writing_Output &bit_stuffing_wr, Error_Output &error, Decoder_Output &decoder)
{
    this->input.frame_mounter = &frame_mounter;
    this->input.bit_stuffing_wr = &bit_stuffing_wr;
    this->input.error = &error;
    this->input.decoder = &decoder;
}

bool Frame_Transmitter::check_errors()
{
    if (this->input.error->error_detected == HIGH) { //CONTROLE DE ERRO
        if (this->input.error->error_state == BUS_OFF_CODE) { //Código de erro Bus_Off
            this->state = BUS_OFF__Frame_Transmitter__;
        } 
        else {
            this->count = 0;
            this->output->stuffing_enable = LOW;

            if (this->input.error->error_state == ACTIVE_ERROR_CODE) { //Código de erro ativo
                this->state = SEND_ACTIVE_ERROR__Frame_Transmitter__;
            }
            else if (this->input.error->error_state == PASSIVE_ERROR_CODE) { //Código de erro passivo
                this->state = SEND_PASSIVE_ERROR__Frame_Transmitter__;
            }            
        }
    }

    return this->input.error->error_detected;
}

void Frame_Transmitter::run()
{
    switch (this->state)
    {
        case INIT__Frame_Transmitter__:
        {
            this->count = 0;
            this->output->arb_output = HIGH;
            this->output->stuffing_enable = LOW;

            if (!this->check_errors()) {
                if (this->input.bit_stuffing_wr->arb_wp == HIGH) {
                    if (this->input.decoder->ack == LOW) {
                        this->state = ACK__Frame_Transmitter__;
                    }
                    else if (this->input.frame_mounter->frame_ready == HIGH) {
                        this->output->stuffing_enable = HIGH;
                        this->output->arb_output = FRAME[this->count];
                        this->count++;
                        this->output->lost_arbitration = LOW;
                        this->output->eof = LOW;

                        if (IDE == 0) {
                            this->state = ARBITRATION_PHASE_STD__Frame_Transmitter__;
                        }
                        else {
                            this->state = ARBITRATION_PHASE_EXT__Frame_Transmitter__;
                        }        
                    }
                }
            }
            
            break;
        }
        case ACK__Frame_Transmitter__:
        {
            this->output->arb_output = this->input.decoder->ack;

            if (this->input.bit_stuffing_wr->arb_wp == HIGH) {
                this->state = INIT__Frame_Transmitter__;
            }
            break;
        }
        case ARBITRATION_PHASE_STD__Frame_Transmitter__:
        {
            if (!this->check_errors()) {
                if (this->input.bit_stuffing_wr->arb_wp == HIGH) {
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                }
                
                if(new_sp == 1 && new_input != this->output->arb_output) {
                    this->output->lost_arbitration = HIGH;
                    this->state = INIT__Frame_Transmitter__;
                    break;
                }
                if(this->count == 13) {
                    for (int i = 15; i <= 18; i++) {
                        dlc[i-15] = FRAME[i];
                    }
                    data_limit = 18 + 8*int(dlc); //ver a função pra transformar o dlc pra inteiro em C++
                    this->state = STANDARD__Frame_Transmitter__;
                    break;
                }
            }
            
            break;
        }
        case ARBITRATION_PHASE_EXT__Frame_Transmitter__: 
        {
            if (!this->check_errors()) {
                if(this->input.bit_stuffing_wr->arb_wp == 1) {
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                }
                
                if(new_sp == 1 && new_input != this->output->arb_output) {
                    this->output->lost_arbitration = 1;
                    this->state = INIT__Frame_Transmitter__;
                    break;
                }
                
                if(this->count == 33) {
                    for (int i = 35; i <= 38; i++) {
                        dlc[i-35] = FRAME[i];
                    }
                    data_limit = 38 + 8*int(dlc); //ver a função pra transformar o dlc pra inteiro em C++
                    this->state = EXTENDED__Frame_Transmitter__;
                    break;
                }
            }        
            break;
        }
        case STANDARD__Frame_Transmitter__:
        {
            if (!this->check_errors()) {
                if(this->input.bit_stuffing_wr->arb_wp == 1) {
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                }

                if(new_sp == 1 && new_input != this->output->arb_output && this->count != data_limit + 16) { //this->count != ack slot
                    this->state = BIT_ERROR__Frame_Transmitter__;
                    break;
                }

                if(this->count == data_limit + 16) {
                    this->output->stuffing_enable = 0;
                }

                if(this->count == data_limit + 29) { //this->count != ack slot
                    this->output->eof = 1;
                    this->state = INIT__Frame_Transmitter__;
                }          
            }        
            break;
        }
        case EXTENDED__Frame_Transmitter__:
        {
            if (!this->check_errors()) {
                if(this->input.bit_stuffing_wr->arb_wp == 1) {
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                }

                if(new_sp == 1 && new_input != this->output->arb_output && this->count != data_limit + 16) { //this->count != ack slot
                    this->state = BIT_ERROR__Frame_Transmitter__;
                    break;
                }

                if(this->count == data_limit + 16) {
                    this->output->stuffing_enable = 0;
                }

                if(this->count == data_limit + 29) { //this->count != ack slot
                    eof = 1;
                    this->state = INIT__Frame_Transmitter__;
                }
            }
            break;
        }
        case BIT_ERROR__Frame_Transmitter__:
        {
            this->output->bit_error = HIGH;
            this->check_errors();
            break;
        }
        case SEND_ACTIVE_ERROR__Frame_Transmitter__:
        {
            this->output->arb_output = 0;

            if(this->count < 6 && this->input.bit_stuffing_wr->arb_wp == 1) {
                this->count++;
            }
            else if(this->count == 6 && this->input.bit_stuffing_wr->arb_wp == 1) {
                this->output->arb_output = 1;
                this->state = INIT__Frame_Transmitter__;
            }
            break;
        }
        case SEND_PASSIVE_ERROR__Frame_Transmitter__:
        {
            this->output->arb_output = 1;

            if(this->count < 6 && this->input.bit_stuffing_wr->arb_wp == 1) {
                this->count++;
            }
            else if(this->count == 6 && this->input.bit_stuffing_wr->arb_wp == 1) {
                this->output->arb_output = 1;
                this->state = INIT__Frame_Transmitter__;
            }
            break;
        }
        case BUS_OFF__Frame_Transmitter__:
        {
            printf("WAIT SYSTEM RESET!!");
            break;
        }
    }
}