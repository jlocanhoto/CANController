#include "config.h"
#include "Frame_Transmitter.h"
#include "Error.h"

Frame_Transmitter::Frame_Transmitter(Frame_Transmitter_Data &output)
{
    output.lost_arbitration = LOW;
    output.arb_output = RECESSIVE;
    output.bit_error = LOW;
    output.ack_error = LOW;
    output.ack_ok_rx = LOW;
    output.ack_ok_tx = LOW;
    output.bus_on = LOW;
    output.stuffing_enable = LOW;
    output.EoF = HIGH;
    this->previous_arb_wr_pt = LOW;
    this->previous_sample_pt = LOW;
    this->previous_ack_slot = LOW;
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
            this->output->bit_error = LOW;
            this->output->ack_error = LOW;
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
    bool writing_point_edge = false;
    bool sample_point_edge = false;
    bool ack_slot_edge = false;
    
    if (this->previous_arb_wr_pt == LOW && this->input.bit_stuffing_wr->arb_wr_pt == HIGH) {
        writing_point_edge = true;
        //Serial.println("FT->writing_point_edge");
    }

    if (this->previous_sample_pt == LOW && this->input.bit_stuffing_rd->new_sample_pt == HIGH) {
        sample_point_edge = true;
        //Serial.println("FT->writing_point_edge");
    }

    if (this->previous_ack_slot == LOW && this->input.decoder->ack_slot == HIGH) {
        ack_slot_edge = true;
        //Serial.println("FT->writing_point_edge");
    }

    switch (this->state)
    {
        case INIT__Frame_Transmitter__:
        {
            //Serial.println("\nINIT__Frame_Transmitter__");
            this->count = 0;
            this->output->arb_output = RECESSIVE;
            this->output->stuffing_enable = LOW;

            if (!this->check_errors()) {
                if (writing_point_edge) {
                    this->output->ack_ok_rx = this->output->ack_ok_tx = LOW;

                    if (ack_slot_edge) {
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
            Serial.println("\nACK__Frame_Transmitter__");            

            if (writing_point_edge) {
                this->output->arb_output = this->input.decoder->ack;
                
                if (this->input.decoder->ack == RECESSIVE) {
                    this->output->ack_error = HIGH;
                    this->state = ACK_ERROR__Frame_Transmitter__;
                }
            }

            if (sample_point_edge) {
                if (this->input.bit_stuffing_rd->new_sampled_bit == this->input.decoder->ack) {
                    this->output->ack_ok_rx = HIGH;
                    this->state = INIT__Frame_Transmitter__;
                }
                else {
                    this->state = BIT_ERROR__Frame_Transmitter__;
                }                
            }
            break;
        }
        case ACK_ERROR__Frame_Transmitter__:
        {
            Serial.println("\nACK_ERROR__Frame_Transmitter__");
            this->output->ack_error = HIGH;
            this->check_errors();
            break;
        }
        case ARBITRATION_PHASE__Frame_Transmitter__:
        {
            //Serial.println("\nARBITRATION_PHASE__Frame_Transmitter__");
            if (!this->check_errors()) {
                if (writing_point_edge) {
                    this->output->arb_output = this->input.frame_mounter->FRAME[this->count];
                    //Serial.print(this->output->arb_output, DEC);
                    this->count++;
                }
                
                if (sample_point_edge && this->input.bit_stuffing_rd->new_sampled_bit != this->output->arb_output) {
                    this->output->lost_arbitration = HIGH;
                    this->state = INIT__Frame_Transmitter__;
                }
                else if (this->count == this->input.frame_mounter->arb_limit) {
                    //Serial.println();
                    this->state = DATA_TO_END_PHASE__Frame_Transmitter__;
                }
            }
            
            break;
        }
        case DATA_TO_END_PHASE__Frame_Transmitter__:
        {
            //Serial.println("\nDATA_TO_END_PHASE__Frame_Transmitter__");
            if (!this->check_errors()) {
                if (writing_point_edge) {
                    //Serial.println(this->count, DEC);
                    if (this->count < this->input.frame_mounter->data_limit + IFS_OFFSET) {
                        this->output->arb_output = this->input.frame_mounter->FRAME[this->count];
                    }
                    else {
                        this->output->arb_output = RECESSIVE;
                    }
                    
                    this->count++;
                }

                if (sample_point_edge && this->input.bit_stuffing_rd->new_sampled_bit != this->output->arb_output
                    && this->count != this->input.frame_mounter->data_limit + ACK_SLOT_OFFSET + 1
                    && this->count < this->input.frame_mounter->data_limit + IFS_OFFSET + 1) { // this->count != ack slot
                    //Serial.println('.');
                    this->state = BIT_ERROR__Frame_Transmitter__;
                }
                else if (sample_point_edge && this->input.bit_stuffing_rd->new_sampled_bit == this->output->arb_output
                    && this->count == this->input.frame_mounter->data_limit + ACK_SLOT_OFFSET + 1) { // this->count != ack slot
                    //Serial.println("ack_ok_tx");
                    this->output->ack_ok_tx = HIGH;
                }
                else if (this->count == this->input.frame_mounter->data_limit + CRC_DELIM_OFFSET + 1) {
                    //Serial.println(',');
                    this->output->stuffing_enable = LOW;
                }
                else if (this->count == this->input.frame_mounter->data_limit + IFS_OFFSET + IFS_SIZE + 1) {
                    //Serial.println(';');
                    this->output->EoF = HIGH;
                    this->state = INIT__Frame_Transmitter__;
                }
            }
            break;
        }
        case BIT_ERROR__Frame_Transmitter__:
        {
            Serial.println("\nBIT_ERROR__Frame_Transmitter__");
            this->output->bit_error = HIGH;
            this->check_errors();
            break;
        }
        case SEND_ACTIVE_ERROR__Frame_Transmitter__:
        {
            Serial.println("\nSEND_ACTIVE_ERROR__Frame_Transmitter__");
            this->output->arb_output = DOMINANT;

            if (writing_point_edge) {
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
            Serial.println("\nSEND_PASSIVE_ERROR__Frame_Transmitter__");
            this->output->arb_output = RECESSIVE;

            if (writing_point_edge) {
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
            Serial.println("\nBUS_OFF__Frame_Transmitter__");
            break;
        }
    }

    this->previous_arb_wr_pt = this->input.bit_stuffing_wr->arb_wr_pt;
    this->previous_sample_pt = this->input.bit_stuffing_rd->new_sample_pt;
    this->previous_ack_slot = this->input.decoder->ack_slot;
}