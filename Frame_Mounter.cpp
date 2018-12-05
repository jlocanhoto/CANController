#include "Frame_Mounter.h"

Frame_Mounter::Frame_Mounter(Frame_Mounter_Data &output, uint16_t max_frame_size)
{
    output.FRAME = (bool*) malloc(max_frame_size * sizeof(bool));
    output.frame_ready = LOW;
    output.data_limit = 0;
    output.arb_limit = 0;

    this->previous_new_frame_signal = LOW;
    this->output = &output;
}

void Frame_Mounter::connect_inputs(Application_Data& application, CRC_Data &crc_interface)
{
    this->input.application = &application;
    this->input.crc_interface = &crc_interface;
}

void Frame_Mounter::run()
{
    bool new_frame_edge = false;

    if (this->previous_new_frame_signal == LOW && this->input.application->new_frame == HIGH) {
        new_frame_edge = true;
    }

    switch(this->state)
    {
        case INIT__Frame_Mounter__:
        {
            //Serial.println("INIT__Frame_Mounter__");
            if (new_frame_edge) {
                this->output->frame_ready = LOW;
                reset_CRC(this->input.crc_interface);
                
                if (this->input.application->output_frame.IDE == DOMINANT) {
                    this->output->arb_limit = RTR_SRR_POS+1;
                    this->state = BASE_FORMAT__Frame_Mounter__;
                }
                else {
                    this->output->arb_limit = RTR_EXT_POS+1;
                    this->state = EXTENDED_FORMAT__Frame_Mounter__;
                }
            }
            break;
        }
        case BASE_FORMAT__Frame_Mounter__:
        {
            //Serial.println("BASE_FORMAT__Frame_Mounter__");
            this->output->FRAME[SOF_POS] = DOMINANT; // SOF

            for (int i = 0; i < ID_A_SIZE; i++) { // ID
                this->output->FRAME[ID_A_POS + i] = ((this->input.application->output_frame.ID << i) & 0x400) > 0;
            }

            this->output->FRAME[RTR_SRR_POS] = this->input.application->output_frame.RTR;
            this->output->FRAME[IDE_POS] = this->input.application->output_frame.IDE;
            this->output->FRAME[r0_POS] = DOMINANT; // r0
            
            for (int i = 0; i < DLC_SIZE; i++) { // DLC
                this->output->FRAME[DLC_POS + i] = ((this->input.application->output_frame.PAYLOAD_SIZE << i) & 0x8) > 0;
            }

            this->input.crc_interface->PT_COUNTER += (DLC_POS + DLC_SIZE);

            if (this->input.application->output_frame.RTR == DOMINANT) {
                this->start_data = DATA_FIELD_POS;
                this->data_limit = DATA_FIELD_POS + this->input.application->output_frame.PAYLOAD_SIZE * 8; //ver a função pra transformar o DLC pra inteiro em C++
                this->state = DATA_FIELD__Frame_Mounter__;
            }
            else { // this->input.application->output_frame.RTR == RECESSIVE
                this->data_limit = DATA_FIELD_POS;
                
                this->input.crc_interface->crc_req = HIGH;

                this->state = CRC_WAIT__Frame_Mounter__;
            }

            this->output->data_limit = this->data_limit;
            break;
        }
        case EXTENDED_FORMAT__Frame_Mounter__:
        {
            //Serial.println("EXTENDED_FORMAT__Frame_Mounter__");
            this->output->FRAME[SOF_POS] = DOMINANT; // SOF
            
            for (int i = 0; i < ID_A_SIZE; i++) { // ID
                this->output->FRAME[ID_A_POS + i] = ((this->input.application->output_frame.ID << i) & 0x10000000) > 0;
            }

            this->output->FRAME[RTR_SRR_POS] = RECESSIVE; // SRR
            this->output->FRAME[IDE_POS] = this->input.application->output_frame.IDE;

            for (int i = 0; i < ID_B_SIZE; i++) { // ID
                this->output->FRAME[ID_B_POS + i] = ((this->input.application->output_frame.ID << i) & 0x20000) > 0;
            }

            this->output->FRAME[RTR_EXT_POS] = this->input.application->output_frame.RTR; 
            this->output->FRAME[r1_POS] = DOMINANT; // r1
            this->output->FRAME[r0_EXT_POS] = DOMINANT; // r0

            for (int i = 0; i < DLC_SIZE; i++) { // DLC
                this->output->FRAME[DLC_EXT_POS + i] = ((this->input.application->output_frame.PAYLOAD_SIZE << i) & 0x8) > 0;
            }

            this->input.crc_interface->PT_COUNTER += (DLC_EXT_POS + DLC_SIZE);

            if (this->input.application->output_frame.RTR == DOMINANT) {
                this->start_data = DATA_FIELD_EXT_POS;
                this->data_limit = DATA_FIELD_EXT_POS + min(this->input.application->output_frame.PAYLOAD_SIZE, 8) * 8; //ver a função pra transformar o DLC pra inteiro em C++
                this->state = DATA_FIELD__Frame_Mounter__;
            }
            else { // this->input.application->output_frame.RTR == RECESSIVE
                this->data_limit = DATA_FIELD_EXT_POS;

                this->input.crc_interface->crc_req = HIGH;

                this->state = CRC_WAIT__Frame_Mounter__;
            }

            this->output->data_limit = this->data_limit;
            break;
        }
        case DATA_FIELD__Frame_Mounter__:
        {
            //Serial.println("DATA_FIELD__Frame_Mounter__");
            for (int i = this->data_limit-1, payload_idx = 0; i >= this->start_data; i--, payload_idx++) { // PAYLOAD
                this->output->FRAME[i] = ((this->input.application->output_frame.PAYLOAD >> payload_idx) & 0x01) > 0;
            }

            //Serial.println(this->input.crc_interface->PT_COUNTER, DEC);

            this->input.crc_interface->PT_COUNTER += (this->data_limit - this->start_data);
            //Serial.println(this->input.crc_interface->PT_COUNTER, DEC);

            this->input.crc_interface->crc_req = HIGH;
            this->state = CRC_WAIT__Frame_Mounter__;
            break;
        }
        case CRC_WAIT__Frame_Mounter__:
        {
            //Serial.println("CRC_WAIT__Frame_Mounter__");
            if (this->input.crc_interface->crc_ready) {
                this->state = CRC_ACK_EOF__Frame_Mounter__;
                this->input.crc_interface->crc_req = LOW;
            }
            break;
        }
        case CRC_ACK_EOF__Frame_Mounter__:
        {
            //Serial.println("CRC_ACK_EOF__Frame_Mounter__");
            for (int i = 0; i < CRC_SIZE; i++) {
                this->output->FRAME[this->data_limit + i] = ((this->input.crc_interface->CRC << i) & 0x4000) > 0;
            }

            this->output->FRAME[this->data_limit + CRC_DELIM_OFFSET] = RECESSIVE; //CRC delimiter
            this->output->FRAME[this->data_limit + ACK_SLOT_OFFSET] = this->input.application->ACK_slot; //ACK slot            
            this->output->FRAME[this->data_limit + ACK_DELIM_OFFSET] = RECESSIVE; //ACK delimiter            

            for (int i = this->data_limit + EOF_OFFSET; i < this->data_limit + EOF_OFFSET + EOF_SIZE; i++) {
                this->output->FRAME[i] = RECESSIVE; // EOF
            }

            this->state = INIT__Frame_Mounter__;
            this->output->frame_ready = HIGH;
            break;
        }
    }
    
    this->previous_new_frame_signal = this->input.application->new_frame;
}

/*
void Frame_Mounter::mount_frame(Splitted_Frame this->input.application->output_frame)
{
    if (IDE == DOMINANT) {
        // base format
        this->frame = (CAN_Frame_Base*) malloc(sizeof(CAN_Frame_Base));
        CAN_Frame_Base* frame = (CAN_Frame_Base*) this->frame;
    
        frame->ID = this->input.application->output_frame.ID;
        
        // common to both formats
        frame->SOF = DOMINANT;
        frame->RTR = this->input.application->output_frame.RTR;
        frame->IDE = this->input.application->output_frame.IDE;
        frame->r0 = DOMINANT;
        frame->DLC = this->input.application->output_frame.PAYLOAD_SIZE;
        frame->Data = this->input.application->output_frame.PAYLOAD;
        frame->CRC = calculate_CRC();
        frame->CRC_delim = RECESSIVE;
        frame->ACK_slot = this->input.application->output_frame.ACK_slot;
        frame->ACK_delim = RECESSIVE;
        frame->EoF = B1111111;
    }
    else {
        // extended format
        this->frame = (CAN_Frame_Ext*) malloc(sizeof(CAN_Frame_Ext));
        CAN_Frame_Ext* frame = (CAN_Frame_Ext*) this->frame;

        frame->ID_A = ID >> 18;
        frame->ID_B = ID & 0x3FFFF; // 0x3FFFF = 18 bits 1
        frame->SRR = RECESSIVE;
        frame->r1 = DOMINANT;

        // common to both formats
        frame->SOF = DOMINANT;
        frame->RTR = this->input.application->output_frame.RTR;
        frame->IDE = this->input.application->output_frame.IDE;
        frame->r0 = DOMINANT;
        frame->DLC = this->input.application->output_frame.PAYLOAD_SIZE;
        frame->Data = this->input.application->output_frame.PAYLOAD;
        frame->CRC = calculate_CRC();
        frame->CRC_delim = RECESSIVE;
        frame->ACK_slot = this->input.application->output_frame.ACK_slot;
        frame->ACK_delim = RECESSIVE;
        frame->EoF = B1111111;
    }
}

void Frame_Mounter::print_frame()
{
    if (((CAN_Frame_Base*) this->frame)->IDE == DOMINANT) {
        // base format
        CAN_Frame_Base* frame = (CAN_Frame_Base*) this->frame;
    
        //Serial.print("SOF = ");
        //Serial.println(frame->SOF, BIN);

        //Serial.print("ID = ");
        //Serial.print(frame->ID, DEC);
        //Serial.print(" <=> ");
        //Serial.println(frame->ID, BIN);

        //Serial.print("RTR = ");
        //Serial.println(frame->RTR, BIN);

        //Serial.print("IDE = ");
        //Serial.println(frame->IDE, BIN);

        //Serial.print("r0 = ");
        //Serial.println(frame->r0, BIN);

        //Serial.print("DLC = ");
        //Serial.print(frame->DLC, DEC);
        //Serial.print(" <=> ");
        //Serial.println(frame->DLC, BIN);

        //Serial.print("Data = ");
        uint32_t data_low = frame->Data % 0xFFFFFFFF; 
        uint32_t data_high = (frame->Data >> 32) % 0xFFFFFFFF;
        print_uint64_t(frame->Data);
        //Serial.print(" <=> ");
        //Serial.print(data_low, BIN);
        //Serial.println(data_high, BIN);

        //Serial.print("CRC = ");
        //Serial.print(frame->CRC, DEC);
        //Serial.print(" <=> ");
        //Serial.println(frame->CRC, BIN);

        //Serial.print("CRC_delim = ");
        //Serial.println(frame->CRC_delim, BIN);

        //Serial.print("ACK_slot = ");
        //Serial.println(frame->ACK_slot, BIN);

        //Serial.print("ACK_delim = ");
        //Serial.println(frame->ACK_delim, BIN);

        //Serial.print("EOF = ");
        //Serial.println(frame->EoF, BIN);
    }
    else {
        // extended format
        CAN_Frame_Ext* frame = (CAN_Frame_Ext*) this->frame;

        //Serial.print("SOF = ");
        //Serial.println(frame->SOF, BIN);

        //Serial.print("ID = ");
        uint32_t id = ((uint32_t) frame->ID_A << 18) | frame->ID_B;
        //Serial.print(id, DEC);
        //Serial.print(" <=> ");
        //Serial.print(" ( ");
        //Serial.print(frame->ID_A, BIN);
        //Serial.print(" ++ ");
        //Serial.print(frame->ID_B, BIN);
        //Serial.println(" )");

        //Serial.print("SRR = ");
        //Serial.println(frame->SRR, BIN);

        //Serial.print("RTR = ");
        //Serial.println(frame->RTR, BIN);

        //Serial.print("IDE = ");
        //Serial.println(frame->IDE, BIN);

        //Serial.print("r1 = ");
        //Serial.println(frame->r1, BIN);

        //Serial.print("r0 = ");
        //Serial.println(frame->r0, BIN);

        //Serial.print("DLC = ");
        //Serial.print(frame->DLC, DEC);
        //Serial.print(" <=> ");
        //Serial.println(frame->DLC, BIN);

        //Serial.print("Data = ");
        uint32_t data_low = frame->Data % 0xFFFFFFFF; 
        uint32_t data_high = (frame->Data >> 32) % 0xFFFFFFFF;
        print_uint64_t(frame->Data);
        //Serial.print(" <=> ");
        //Serial.print(data_low, BIN);
        //Serial.println(data_high, BIN);

        //Serial.print("CRC = ");
        //Serial.print(frame->CRC, DEC);
        //Serial.print(" <=> ");
        //Serial.println(frame->CRC, BIN);

        //Serial.print("CRC_delim = ");
        //Serial.println(frame->CRC_delim, BIN);

        //Serial.print("ACK_slot = ");
        //Serial.println(frame->ACK_slot, BIN);

        //Serial.print("ACK_delim = ");
        //Serial.println(frame->ACK_delim, BIN);
        
        //Serial.print("EOF = ");
        //Serial.println(frame->EoF, BIN);
    }
}
*/