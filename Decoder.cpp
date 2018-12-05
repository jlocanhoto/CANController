#include "Decoder.h"

Decoder::Decoder(Decoder_Data &output, uint16_t partial_frame_size)
{
    output.decoded_frame.IDE = DOMINANT;
    output.decoded_frame.ID = 0;
    output.decoded_frame.PAYLOAD = 0;
    output.decoded_frame.PAYLOAD_SIZE = 0;
    output.decoded_frame.RTR = DOMINANT;

    output.PARTIAL_FRAME = (bool*) malloc(partial_frame_size * sizeof(bool));
    
    output.crc_error = LOW;
    output.format_error = LOW;
    output.EoF = 0;

    output.ack = RECESSIVE;
    output.ack_slot = LOW;
    
    output.stuffing_enable = LOW;

    this->output = &output;
    this->count = 0;
    this->CRC = 0;
    this->previous_EoF_frame_mounter = LOW;
    this->previous_sample_pt = LOW;
    this->state = INIT_SOF__Decoder__;
}

void Decoder::connect_inputs(Bit_Stuffing_Reading_Data &bit_stuffing_rd, CRC_Data &crc_interface, Frame_Transmitter_Data &frame_transmitter)
{
    this->input.bit_stuffing_rd = &bit_stuffing_rd;
    this->input.crc_interface = &crc_interface;
    this->input.frame_transmitter = &frame_transmitter;
}

void Decoder::run()
{
    bool sample_point_edge = false;

    if (this->previous_sample_pt == LOW && this->input.bit_stuffing_rd->new_sample_pt == HIGH) {
        sample_point_edge = true;
    }

    if (this->input.bit_stuffing_rd->stuff_error) {
        this->state = ERROR__Decoder__;
        Serial.println("STUFF_ERROR");
        while(true);
    }

    if (sample_point_edge) {
        switch (this->state)
        {
            case INIT_SOF__Decoder__:
            {
                //Serial.println("INIT__Decoder__");
                this->arb = false;
                this->output->EoF = HIGH;

                if (this->input.bit_stuffing_rd->new_sampled_bit == DOMINANT) {
                    reset_CRC(this->input.crc_interface);
                    this->CRC = 0;

                    //Serial.println("SOF__Decoder__");
                    this->output->EoF = LOW;
                    this->output->stuffing_enable = HIGH;
                    this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                    this->input.crc_interface->PT_COUNTER++;
                    this->count = 0;

                    if (this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                        //Serial.println("\nLost Arbitration\n");
                        this->arb = true;
                    }
                    
                    this->output->decoded_frame.ID = 0;
                    this->state = IDENTIFIER_A__Decoder__;
                }
                
                break;
            }
            case IDENTIFIER_A__Decoder__:
            {
                //Serial.println("IDENTIFIER_A__Decoder__");
                this->output->decoded_frame.ID <<= 1;
                this->output->decoded_frame.ID |= this->input.bit_stuffing_rd->new_sampled_bit;
                this->count++;
                this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->input.crc_interface->PT_COUNTER++;

                if (this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                    this->arb = true;
                }
                
                if (this->count == ID_A_SIZE) {
                    this->state = RTR_SRR__Decoder__;
                }
                break;
            }
            case RTR_SRR__Decoder__: 
            {
                //Serial.println("RTR_SRR__Decoder__");
                this->output->decoded_frame.RTR = this->input.bit_stuffing_rd->new_sampled_bit;
                this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->input.crc_interface->PT_COUNTER++;

                if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                    this->arb = true;
                }

                this->state = IDE__Decoder__;
                break;
            }
            case IDE__Decoder__:
            {
                //Serial.println("IDE__Decoder__");
                this->output->decoded_frame.IDE = this->input.bit_stuffing_rd->new_sampled_bit;
                this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->input.crc_interface->PT_COUNTER++;

                if(this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                    this->arb = true;
                }

                this->count = 0;

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
                //Serial.println("IDENTIFIER_B__Decoder__");
                this->output->decoded_frame.ID <<= 1;
                this->output->decoded_frame.ID |= this->input.bit_stuffing_rd->new_sampled_bit;
                this->count++;
                this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->input.crc_interface->PT_COUNTER++;

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
                //Serial.println("RTR_EXT__Decoder__");
                this->SRR = this->output->decoded_frame.RTR;

                if (this->SRR != RECESSIVE) {
                    this->output->format_error = HIGH;
                    //Serial.println("[ERROR] Format Error (SRR SIZE)");
                    this->state = ERROR__Decoder__;
                    Serial.println("FORMAT_ERROR");
                    while(true);
                }
                else {
                    this->output->decoded_frame.RTR = this->input.bit_stuffing_rd->new_sampled_bit;
                    this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                    this->input.crc_interface->PT_COUNTER++;

                    if (this->input.frame_transmitter->lost_arbitration == HIGH && this->arb == false) {
                        this->arb = true;
                    }

                    this->state = r1__Decoder__;
                }
                break;
            }
            case r1__Decoder__:
            {
                //Serial.println("r1__Decoder__");
                this->r1 = this->input.bit_stuffing_rd->new_sampled_bit;
                this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->input.crc_interface->PT_COUNTER++;
                this->state = r0__Decoder__;
                break;
            }
            case r0__Decoder__:
            {
                //Serial.println("r0__Decoder__");
                this->r0 = this->input.bit_stuffing_rd->new_sampled_bit;
                this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->input.crc_interface->PT_COUNTER++;
                this->output->decoded_frame.PAYLOAD_SIZE = 0;

                if (this->arb) {
                    this->state = STAND_BY__Decoder__;
                }
                else {
                    this->count = 0;
                    this->state = DLC__Decoder__;
                }
                break;
            }
            case STAND_BY__Decoder__:
            {
                Serial.println("STAND_BY__Decoder__");
                if (sample_point_edge) {
                    this->input.frame_transmitter->EoF == HIGH;
                    this->state = INIT_SOF__Decoder__;
                }

                break;
            }
            case DLC__Decoder__:
            {
                //Serial.println("DLC__Decoder__");
                this->output->decoded_frame.PAYLOAD_SIZE <<= 1;
                this->output->decoded_frame.PAYLOAD_SIZE |= this->input.bit_stuffing_rd->new_sampled_bit;
                this->count++;

                this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->input.crc_interface->PT_COUNTER++;

                if (this->count == DLC_SIZE) {
                    this->output->decoded_frame.PAYLOAD_SIZE = min(MAX_PAYLOAD_BYTES, this->output->decoded_frame.PAYLOAD_SIZE);
                    this->CRC = 0;
                    this->output->decoded_frame.PAYLOAD = 0;

                    if ((this->output->decoded_frame.RTR == DOMINANT) && (this->output->decoded_frame.PAYLOAD_SIZE > 0)) {
                        this->count = 0;
                        this->state = DATA_FIELD__Decoder__;
                        
                    }
                    else { // if ((this->output->decoded_frame.RTR == RECESSIVE) || this->output->decoded_frame.PAYLOAD_SIZE == 0))
                        this->count = 0;
                        this->input.crc_interface->crc_req = HIGH;
                        this->state = CRC__Decoder__;
                    }
                }

                break;
            }
            case DATA_FIELD__Decoder__:
            {
                //Serial.println("DATA_FIELD__Decoder__");
                this->output->decoded_frame.PAYLOAD <<= 1;
                this->output->decoded_frame.PAYLOAD |= this->input.bit_stuffing_rd->new_sampled_bit;
                this->count++;

                this->output->PARTIAL_FRAME[this->input.crc_interface->PT_COUNTER] = this->input.bit_stuffing_rd->new_sampled_bit;
                this->input.crc_interface->PT_COUNTER++;

                if (this->count == (this->output->decoded_frame.PAYLOAD_SIZE * 8)) {
                    this->count = 0;
                    this->input.crc_interface->crc_req = HIGH;
                    this->state = CRC__Decoder__;
                }
                break;
            }
            case CRC__Decoder__:
            {
                //Serial.println("CRC__Decoder__");
                this->CRC <<= 1;
                this->CRC |= this->input.bit_stuffing_rd->new_sampled_bit;
                this->count++;

                if (this->count == CRC_SIZE) {
                    this->output->stuffing_enable = LOW;
                    this->state = CRC_delimiter__Decoder__;
                    //Serial.println(this->CRC, BIN);
                }
                break;
            }
            case CRC_delimiter__Decoder__:
            {
                //Serial.println("CRC_delimiter__Decoder__");
                if (this->input.crc_interface->crc_ready) {
                    this->input.crc_interface->crc_req = LOW;
                    this->crc_ok = (this->CRC == this->input.crc_interface->CRC);
                    this->CRC_delim = RECESSIVE;
                    this->state = ACK_slot__Decoder__;

                    this->output->ack = !this->crc_ok;
                    this->output->ack_slot = HIGH;
                }

                break;
            }
            case ACK_slot__Decoder__:
            {
                //Serial.println("ACK_slot__Decoder__");
                this->ACK_slot = this->output->ack;
                this->state = ACK_delimiter__Decoder__;

                break;
            }
            case ACK_delimiter__Decoder__:
            {
                if (this->output->ack == RECESSIVE) {
                    this->output->crc_error = HIGH;
                    //Serial.println("[ERROR] CRC Error");
                    this->state = ERROR__Decoder__;
                    Serial.println("CRC_ERROR");
                    while(true);
                }

                this->output->ack = RECESSIVE;
                this->output->ack_slot = LOW;
                //Serial.println("ACK_delimiter__Decoder__");
                bool ack_d = this->input.bit_stuffing_rd->new_sampled_bit;

                if (ack_d == DOMINANT) {
                    this->output->format_error = HIGH;
                    this->state = ERROR__Decoder__;
                    Serial.println("FORMAT_ERROR");
                    while(true);
                }
                else { // ack_d == RECESSIVE
                    this->ACK_delim = ack_d;
                    this->count = 0;
                    this->state = EOF__Decoder__;
                }
                break;
            }
            case EOF__Decoder__:
            {
                //Serial.println("EOF__Decoder__");
                this->count++;

                if (this->count < EOF_SIZE) {
                    if (this->input.bit_stuffing_rd->new_sampled_bit == DOMINANT) {
                        this->output->format_error = HIGH;
                        //Serial.println("[ERROR] Format Error (EOF SIZE)");
                        this->state = ERROR__Decoder__;
                        Serial.println("FORMAT_ERROR");
                        while(true);
                    }
                    else {
                        this->EoF <<= 1;
                        this->EoF |= RECESSIVE;
                    }
                }
                else {
                    this->state = INIT_SOF__Decoder__;
                    this->EoF <<= 1;
                    this->EoF |= RECESSIVE;

                    this->print_frame();
                }
                break;
            }
            case ERROR__Decoder__:
            {
                Serial.println("ERROR__Decoder__");
                break;
            }
        }
    }

    this->previous_sample_pt = this->input.bit_stuffing_rd->new_sample_pt;
    this->previous_EoF_frame_mounter = this->output->EoF;
}

void Decoder::print_frame()
{
    Serial.println("[DECODED FRAME]");

    Serial.print("SOF = ");
    Serial.println(this->SOF, BIN);

    Serial.print("ID = 0x");
    Serial.print(this->output->decoded_frame.ID, HEX);
    Serial.print(" <=> ");
    Serial.println(this->output->decoded_frame.ID, BIN);
    
    if (this->output->decoded_frame.IDE == RECESSIVE) {
        Serial.print("SRR = ");
        Serial.println(this->SRR, BIN);
    }
    else {
        Serial.print("RTR = ");
        Serial.println(this->output->decoded_frame.RTR, BIN);
    }

    Serial.print("IDE = ");
    Serial.println(this->output->decoded_frame.IDE, BIN);

    if (this->output->decoded_frame.IDE == RECESSIVE) {
        Serial.print("RTR = ");
        Serial.println(this->output->decoded_frame.RTR, BIN);

        Serial.print("r1 = ");
        Serial.println(this->r1, BIN);
    }

    Serial.print("r0 = ");
    Serial.println(this->r0, BIN);

    Serial.print("DLC = ");
    Serial.print(this->output->decoded_frame.PAYLOAD_SIZE, DEC);
    Serial.print(" <=> ");
    Serial.println(this->output->decoded_frame.PAYLOAD_SIZE, BIN);

    Serial.print("Data = ");
    uint32_t data_low = this->output->decoded_frame.PAYLOAD & 0xFFFFFFFF; 
    uint32_t data_high = (this->output->decoded_frame.PAYLOAD >> 32) & 0xFFFFFFFF;
    print_uint64_t(this->output->decoded_frame.PAYLOAD);
    Serial.print(" <=> 0x");
    Serial.print(data_high, HEX);
    Serial.print(data_low, HEX);
    Serial.print(" <=> ");
    Serial.print(data_high, BIN);
    Serial.println(data_low, BIN);

    Serial.print("CRC = ");
    Serial.print(this->CRC, DEC);
    Serial.print(" <=> ");
    Serial.println(this->CRC, BIN);

    Serial.print("CRC_delim = ");
    Serial.println(this->CRC_delim, BIN);

    Serial.print("ACK_slot = ");
    Serial.println(this->ACK_slot, BIN);

    Serial.print("ACK_delim = ");
    Serial.println(this->ACK_delim, BIN);

    Serial.print("EOF = ");
    Serial.println(this->EoF, BIN);
}