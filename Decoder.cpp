#include "config.h"
#include "Decoder.h"

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
    this->state = INIT__Decoder__;
    this->count = 0;
}

void Decoder::connect_inputs(Bit_Stuffing_Reading_Data &bit_stuffing_rd, CRC_Calculator_Data &crc_calc, Frame_Transmitter_Data &frame_transmitter)
{
    this->input.bit_stuffing_rd = &bit_stuffing_rd;
    this->input.crc_calc = &crc_calc;
    this->input.frame_transmitter = &frame_transmitter;
}

void Decoder::run()
{
    switch (this->state)
    {
        case INIT__Decoder__:
        {
            this->arb = false;

            if (sampled_bit == 0) {
                output.PT_COUNTER = 0;
                this->state = SOF__Decoder__;
            }
            
            break;
        }
        case SOF__Decoder__:
        {
            this->output->EoF = LOW;
            stuffing_enable = 1;
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;
            this->count = 0;

            if (this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }
            
            this->state = IDENTIFIER_A__Decoder__;
            break;
        }
        case IDENTIFIER_A__Decoder__:
        {
            ID[this->count] = sampled_bit;
            this->count++;
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }
            
            if(this->count == 11) {
                this->state = RTR_SRR__Decoder__;
            }
            break;
        }
        case RTR_SRR__Decoder__: 
        {
            rtr = sampled_bit;
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }

            this->state = IDE__Decoder__;
            break;
        }
        case IDE__Decoder__:
        {
            ide = sampled_bit;
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }

            if(ide == 1) {
                this->state = IDENTIFIER_B__Decoder__;
            }
            else if(ide == 0) {
                this->state = r0__Decoder__;
            }
            break;
        }
        case IDENTIFIER_B__Decoder__:
        {
            ID[this->count] = sampled_bit;
            this->count++;
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }

            if(this->count == 30) {
                this->state = RTR_EXT__Decoder__;
            }
            break;
        }
        case RTR_EXT__Decoder__:
        {
            srr = rtr;
            rtr = sampled_bit;
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;

            if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                this->arb = true;
            }

            this->state = r1__Decoder__;
            break;
        }
        case r1__Decoder__:
        {
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;
            this->state = r0__Decoder__;
            break;
        }
        case r0__Decoder__:
        {
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;

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
            PAYLOAD_SIZE[this->count] = sampled_bit;
            this->count++;
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;

            if(this->count > 3 && rtr == 0) {
                this->count = 0;
                PAYLOAD_SIZE = min(PAYLOAD_SIZE, 8); //ver função em c++ para fazer a conversão de binário p/ decimal e vice-versa
                this->state = DATA_FIELD__Decoder__;
            }
            else if(this->count > 3 && rtr == 1) {
                this->count = 0;
                this->state = CRC__Decoder__;
            }
            break;
        }
        case DATA_FIELD__Decoder__:
        {
            PAYLOAD[this->count] = sampled_bit;
            this->count++;
            output.PARTIAL_FR[output.PT_COUNTER] = sampled_bit;
            output.PT_COUNTER++;

            if(this->count > (int(PAYLOAD_SIZE) * 8) - 1) { //ver função em c++ que converte binário para decimal
                this->count = 0;
                crc_req = 1;
                this->state = CRC__Decoder__;
            }
            break;
        }
        case CRC__Decoder__:
        {
            crc[this->count] = sampled_bit;
            this->count++;

            if(this->count == 15) {
                crc_req = 0;
                stuffing_enable = 0;
                this->state = CRC_delimiter__Decoder__;
            }
            break;
        }
        case CRC_delimiter__Decoder__:
        {
            if(crc_ready == 1){
                if(crc == calc_crc) { //comparar 2 arrays em c++
                        crc_ok = 1;
                }
                else {
                    crc_ok = 0;
                }
                this->state = ACK_slot__Decoder__;
            }
            break;
        }
        case ACK_slot__Decoder__:
        {
            ack = !crc_ok;

            if(ack == 1) {
                this->output->crc_error = 1;
                this->state = ERROR__Decoder__;
            }
            else if(ack == 0) {
                this->state = ACK_delimiter__Decoder__;
            }
            break;
        }
        case ACK_delimiter__Decoder__:
        {
            ack_d = sampled_bit;

            if(ack_d == 0) {
                this->output->format_error = 1;
                this->state = ERROR__Decoder__;
            }
            else if(ack_d == 1) {
                this->count = 0;
                this->state = EOF__Decoder__;
            }
            break;
        }
        case EOF__Decoder__:
        {
            this->count++;
            if (this->count < 7 && sampled_bit == 0)
            {
                this->output->format_error = 1;
                this->state = ERROR__Decoder__;
            }
            else if (this->count == 7) {
                this->output->EoF = HIGH;
                this->state = INIT__Decoder__;
            }
            break;
        }
    }
}