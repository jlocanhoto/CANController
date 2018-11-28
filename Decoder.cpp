#include "config.h"
#include "Decoder.h"
#include "frame_positions.h"

Decoder::Decoder(Decoder_Data &output, uint16_t partial_frame_size)
{
    output.decoded_frame.IDE = DOMINANT;
    output.decoded_frame.ID = 0;
    output.decoded_frame.PAYLOAD = 0;
    output.decoded_frame.PAYLOAD_SIZE = 0;
    output.decoded_frame.RTR = DOMINANT;

    output.crc_req = LOW;
    output.PT_COUNTER = 0;
    output.PARTIAL_FR = (bool*) malloc(partial_frame_size * sizeof(bool));
    
    output.crc_error = LOW;
    output.format_error = LOW;
    output.EoF = LOW;

    output.ack = RECESSIVE;
    
    output.stuffing_enable = LOW;

    this->output = &output;
    this->count = 0;
    this->CRC = 0;
    this->state = INIT__Decoder__;
}

void Decoder::connect_inputs(Bit_Stuffing_Reading_Data &bit_stuffing_rd, CRC_Data &crc_interface, Frame_Transmitter_Data &frame_transmitter)
{
    this->input.bit_stuffing_rd = &bit_stuffing_rd;
    this->input.crc_interface = &crc_interface;
    this->input.frame_transmitter = &frame_transmitter;
}

void Decoder::run()
{
    switch (this->state)
    {
        case INIT__Decoder__:
        {
            this->arb = false;
            this->output->EoF = HIGH;

            if (this->input.bit_stuffing_rd->new_sampled_bit == 0) {
                this->output->PT_COUNTER = 0;
                this->state = SOF__Decoder__;
            }
            
            break;
        }
        case SOF__Decoder__:
        {
            this->output->EoF = LOW;
            this->output->stuffing_enable = HIGH;
            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;
            this->count = 0;

            if (this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }
            
            this->output->decoded_frame.ID = 0;
            this->state = IDENTIFIER_A__Decoder__;
            break;
        }
        case IDENTIFIER_A__Decoder__:
        {
            this->output->decoded_frame.ID <<= 1;
            this->output->decoded_frame.ID |= this->input.bit_stuffing_rd->new_sampled_bit;
            this->count++;
            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }
            
            if(this->count == ID_A_SIZE) {
                this->state = RTR_SRR__Decoder__;
            }
            break;
        }
        case RTR_SRR__Decoder__: 
        {
            this->output->decoded_frame.RTR = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }

            this->state = IDE__Decoder__;
            break;
        }
        case IDE__Decoder__:
        {
            this->count = 0;
            this->output->decoded_frame.IDE = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }

            if(this->output->decoded_frame.IDE == RECESSIVE) {
                this->state = IDENTIFIER_B__Decoder__;
            }
            else if(this->output->decoded_frame.IDE == DOMINANT) {
                this->state = r0__Decoder__;
            }
            break;
        }
        case IDENTIFIER_B__Decoder__:
        {
            this->output->decoded_frame.ID <<= 1;
            this->output->decoded_frame.ID |= this->input.bit_stuffing_rd->new_sampled_bit;
            this->count++;
            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }

            if (this->count == ID_B_SIZE) {
                this->state = RTR_EXT__Decoder__;
            }
            break;
        }
        case RTR_EXT__Decoder__:
        {
            uint8_t SRR = this->output->decoded_frame.RTR;

            if (SRR != RECESSIVE) {
                this->output->format_error = HIGH;
                this->state = ERROR__Decoder__;
            }
            else {
                this->output->decoded_frame.RTR = this->input.bit_stuffing_rd->new_sampled_bit;
                this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->output->PT_COUNTER++;

                if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                    this->arb = true;
                }

                this->state = r1__Decoder__;
            }
            break;
        }
        case r1__Decoder__:
        {
            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;
            this->state = r0__Decoder__;
            break;
        }
        case r0__Decoder__:
        {
            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;
            this->output->decoded_frame.PAYLOAD_SIZE = 0;

            if(this->arb == false) {
                this->state = STAND_BY__Decoder__;
            }
            else if(this->arb == 1) {
                this->count = 0;
                this->state = DLC__Decoder__;
            }
            break;
        }
        case STAND_BY__Decoder__:
        {
            if (this->input.frame_transmitter->EoF  == HIGH) {
                this->state = INIT__Decoder__;
            }

            break;
        }
        case DLC__Decoder__:
        {
            this->output->decoded_frame.PAYLOAD_SIZE <<= 1;
            this->output->decoded_frame.PAYLOAD_SIZE |= this->input.bit_stuffing_rd->new_sampled_bit;
            this->count++;

            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;

            if (this->count == DLC_SIZE) {
                this->output->decoded_frame.PAYLOAD_SIZE = min(MAX_PAYLOAD_BYTES, this->output->decoded_frame.PAYLOAD_SIZE);
                this->CRC = 0;

                if (this->output->decoded_frame.RTR == DOMINANT) {
                    this->count = 0;
                    this->output->decoded_frame.PAYLOAD = 0;
                    this->state = DATA_FIELD__Decoder__;
                    
                }
                else if (this->output->decoded_frame.RTR == RECESSIVE) {
                    this->count = 0;
                    this->output->crc_req = HIGH;
                    this->state = CRC__Decoder__;
                }
            }
            break;
        }
        case DATA_FIELD__Decoder__:
        {
            this->output->decoded_frame.PAYLOAD <<= 1;
            this->output->decoded_frame.PAYLOAD |= this->input.bit_stuffing_rd->new_sampled_bit;
            this->count++;

            this->output->PARTIAL_FR[this->output->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
            this->output->PT_COUNTER++;

            if (this->count == (this->output->decoded_frame.PAYLOAD_SIZE * 8)) {
                this->count = 0;
                this->output->crc_req = HIGH;
                this->state = CRC__Decoder__;
            }
            break;
        }
        case CRC__Decoder__:
        {
            this->CRC <<= 1;
            this->CRC |= this->input.bit_stuffing_rd->new_sampled_bit;
            this->count++;

            if (this->count == CRC_SIZE) {
                this->output->crc_req = LOW;
                this->output->stuffing_enable = 0;
                this->state = CRC_delimiter__Decoder__;
            }
            break;
        }
        case CRC_delimiter__Decoder__:
        {
            this->crc_ok = (this->CRC == this->input.crc_interface->CRC);
            this->state = ACK_slot__Decoder__;

            break;
        }
        case ACK_slot__Decoder__:
        {
            this->output->ack = !this->crc_ok;

            if (this->output->ack == RECESSIVE) {
                this->output->crc_error = HIGH;
                this->state = ERROR__Decoder__;
            }
            else { // this->output->ack == DOMINANT
                this->state = ACK_delimiter__Decoder__;
            }

            break;
        }
        case ACK_delimiter__Decoder__:
        {
            bool ack_d = this->input.bit_stuffing_rd->new_sampled_bit;

            if (ack_d == DOMINANT) {
                this->output->format_error = HIGH;
                this->state = ERROR__Decoder__;
            }
            else { // ack_d == RECESSIVE
                this->count = 0;
                this->state = EOF__Decoder__;
            }
            break;
        }
        case EOF__Decoder__:
        {
            this->count++;

            if (this->count < EOF_SIZE) {
                if (this->input.bit_stuffing_rd->new_sampled_bit == DOMINANT) {
                    this->output->format_error = HIGH;
                    this->state = ERROR__Decoder__;
                }
            }
            else {
                this->state = INIT__Decoder__;
            }
            break;
        }
    }
}