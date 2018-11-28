#include "config.h"
#include "Frame_Transmitter.h"
#include "Error.h"

Frame_Transmitter::Frame_Transmitter(Frame_Transmitter_Data &output)
{
    output.lost_arbitration = LOW;
    this->output = &output;
    this->state = INIT__Frame_Transmitter__;
}

void Frame_Transmitter::connect_inputs(Frame_Mounter_Data &frame_mounter, Bit_Stuffing_Writing_Data &bit_stuffing_wr, Bit_Stuffing_Reading_Data &bit_stuffing_rd, Error_Data &error, Decoder_Data &decoder)
{
    this->input.frame_mounter = &frame_mounter;
    this->input.bit_stuffing_wr = &bit_stuffing_wr;
    this->input.bit_stuffing_rd = &bit_stuffing_rd;
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
            this->output->arb_output = RECESSIVE;
            this->output->stuffing_enable = LOW;

            if (!this->check_errors()) {
                if (this->input.bit_stuffing_wr->arb_wr_pt == HIGH) {
                    if (this->input.decoder->ack == DOMINANT) {
                        this->state = ACK__Frame_Transmitter__;
                    }
                    else if (this->input.frame_mounter->frame_ready == HIGH) {
                        this->output->stuffing_enable = HIGH;
                        this->output->arb_output = this->input.frame_mounter->FRAME[this->count];
                        this->count++;
                        this->output->lost_arbitration = LOW;
                        this->output->EoF = LOW;

                        this->state = ARBITRATION_PHASE__Frame_Transmitter__;
                    }
                }
            }
            
            break;
        }
        case ACK__Frame_Transmitter__:
        {
            this->output->arb_output = this->input.decoder->ack;

            if (this->input.bit_stuffing_wr->arb_wr_pt == HIGH) {
                this->state = INIT__Frame_Transmitter__;
            }
            break;
        }
        case ARBITRATION_PHASE__Frame_Transmitter__:
        {
            if (!this->check_errors()) {
                if (this->input.bit_stuffing_wr->arb_wr_pt == HIGH) {
                    this->output->arb_output = this->input.frame_mounter->FRAME[this->count];
                    this->count++;
                }
                
                if (this->input.bit_stuffing_rd->new_sample_pt == HIGH
                    && this->input.bit_stuffing_rd->new_sampled_bit != this->output->arb_output) {
                    this->output->lost_arbitration = HIGH;
                    this->state = INIT__Frame_Transmitter__;
                }
                else if (this->count == this->input.frame_mounter->arb_limit + 1) {
                    this->state = DATA_TO_END_PHASE__Frame_Transmitter__;
                }
            }
            
            break;
        }
        case DATA_TO_END_PHASE__Frame_Transmitter__:
        {
            if (!this->check_errors()) {
                if (this->input.bit_stuffing_wr->arb_wr_pt == HIGH) {
                    this->output->arb_output = this->input.frame_mounter->FRAME[this->count];
                    this->count++;
                }

                if (this->input.bit_stuffing_rd->new_sample_pt == HIGH
                    && this->input.bit_stuffing_rd->new_sampled_bit != this->output->arb_output
                    && this->count != this->input.frame_mounter->data_limit + ACK_SLOT_OFFSET + 1) { // this->count != ack slot
                    
                    this->state = BIT_ERROR__Frame_Transmitter__;
                }
                else if (this->count == this->input.frame_mounter->data_limit + ACK_SLOT_OFFSET + 1) {
                    this->output->stuffing_enable = LOW;
                }
                else if (this->count == this->input.frame_mounter->data_limit + EOF_OFFSET + EOF_SIZE + IFS_SIZE + 1) {
                    this->output->EoF = HIGH;
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
            this->output->arb_output = DOMINANT;

            if (this->input.bit_stuffing_wr->arb_wr_pt == HIGH) {
                if (this->count < 6) {
                    this->count++;
                }
                else {
                    this->output->arb_output = RECESSIVE;
                    this->state = INIT__Frame_Transmitter__;
                }
            }
            break;
        }
        case SEND_PASSIVE_ERROR__Frame_Transmitter__:
        {
            this->output->arb_output = RECESSIVE;

            if (this->input.bit_stuffing_wr->arb_wr_pt == HIGH) {
                if (this->count < 6) {
                    this->count++;
                }
                else {
                    this->output->arb_output = RECESSIVE;
                    this->state = INIT__Frame_Transmitter__;
                }
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