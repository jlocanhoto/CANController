#include <Arduino.h>
#include "Frame_Mounter.h"
#include "Application.h"
#include "CRC_Calculator.h"
#include "config.h"
#include "utils.h"

Frame_Mounter::Frame_Mounter()
{
    this->frame_ready = LOW;
}

bool Frame_Mounter::mount(bool new_frame, Splitted_Frame &input_frame, CRC_Data &crc_data, bool* FRAME)
{
    static Frame_Mounter_States state = INIT__Frame_Mounter__;
    static uint8_t start_data;
    static uint8_t data_limit;
    static uint8_t pt_counter;

    switch(state)
    {
        case INIT__Frame_Mounter__:
        {
            if (new_frame) {
                this->frame_ready = LOW;
                reset_CRC(crc_data);
                
                if (input_frame.IDE == DOMINANT) {
                    state = BASE_FORMAT__Frame_Mounter__;
                }
                else {
                    state = EXTENDED_FORMAT__Frame_Mounter__;
                }
            }
            break;
        }
        case BASE_FORMAT__Frame_Mounter__:
        {
            FRAME[SOF_POS] = DOMINANT; // SOF

            for (int i = 0; i < ID_A_SIZE; i++) { // ID
                FRAME[ID_A_POS + i] = (input_frame.ID << i) & 0x400;
            }

            FRAME[RTR_SSR_POS] = input_frame.RTR;
            FRAME[IDE_POS] = input_frame.IDE;
            FRAME[r0_POS] = DOMINANT; // r0
            
            for (int i = 0; i < DLC_SIZE; i++) { // DLC
                FRAME[DLC_POS + i] = (input_frame.PAYLOAD_SIZE << i) & 0x8;
            }

            if (input_frame.RTR == DOMINANT) {
                start_data = DATA_FIELD_POS;
                data_limit = DATA_FIELD_POS + input_frame.PAYLOAD_SIZE * 8; //ver a função pra transformar o DLC pra inteiro em C++
                state = DATA_FIELD__Frame_Mounter__;
            }
            else { // input_frame.RTR == RECESSIVE
                data_limit = DATA_FIELD_POS;
                
                crc_data.PT_COUNTER = data_limit;
                crc_data.crc_req = HIGH;

                state = CRC_WAIT__Frame_Mounter__;
            }
            break;
        }
        case EXTENDED_FORMAT__Frame_Mounter__:
        {
            FRAME[SOF_POS] = DOMINANT; // SOF
            
            for (int i = 0; i < ID_A_SIZE; i++) { // ID
                FRAME[ID_A_POS + i] = (input_frame.ID << i) & 0x10000000;
            }

            FRAME[RTR_SSR_POS] = RECESSIVE; // SRR
            FRAME[IDE_POS] = input_frame.IDE;

            for (int i = 0; i < ID_B_SIZE; i++) { // ID
                FRAME[ID_B_POS + i] = (input_frame.ID << i) & 0x20000;
            }

            FRAME[RTR_EXT_POS] = input_frame.RTR; 
            FRAME[r1_POS] = DOMINANT; // r1
            FRAME[r0_EXT_POS] = DOMINANT; // r0

            for (int i = 0; i < DLC_SIZE; i++) { // DLC
                FRAME[DLC_EXT_POS + i] = (input_frame.PAYLOAD_SIZE << i) & 0x8;
            }

            if (input_frame.RTR == DOMINANT) {
                start_data = DATA_FIELD_EXT_POS;
                data_limit = DATA_FIELD_EXT_POS + input_frame.PAYLOAD_SIZE * 8; //ver a função pra transformar o DLC pra inteiro em C++
                state = DATA_FIELD__Frame_Mounter__;
            }
            else { // input_frame.RTR == RECESSIVE
                data_limit = DATA_FIELD_EXT_POS;

                crc_data.PT_COUNTER = data_limit;
                crc_data.crc_req = HIGH;

                state = CRC_WAIT__Frame_Mounter__;
            }
            break;
        }
        case DATA_FIELD__Frame_Mounter__:
        {
            for (int i = data_limit-1, payload_idx = 0; i >= start_data; i--, payload_idx++) { // PAYLOAD
                FRAME[i] = (input_frame.PAYLOAD >> payload_idx) & 0x01;
            }

            crc_data.PT_COUNTER = data_limit;
            crc_data.crc_req = HIGH;
            state = CRC_WAIT__Frame_Mounter__;
            break;
        }
        case CRC_WAIT__Frame_Mounter__:
        {
            if (crc_data.crc_ready) {
                state = CRC_ACK_EOF__Frame_Mounter__;
                crc_data.crc_req = LOW;
            }
            break;
        }
        case CRC_ACK_EOF__Frame_Mounter__:
        {
            for (int i = 0; i < CRC_SIZE; i++) {
                FRAME[data_limit + i] = (crc_data.CRC << i) & 0x4000;
            }

            FRAME[data_limit + CRC_DELIM_OFFSET] = RECESSIVE; //CRC delimiter
            FRAME[data_limit + ACK_SLOT_OFFSET] = input_frame.ACK_slot; //ACK slot
            FRAME[data_limit + ACK_DELIM_OFFSET] = RECESSIVE; //ACK delimiter

            for (int i = data_limit + EOF_OFFSET; i <= data_limit + EOF_OFFSET + EOF_SIZE; i++) {
                FRAME[i] = RECESSIVE; // EOF
            }

            state = INIT__Frame_Mounter__;
            this->frame_ready = HIGH;
            break;
        }
    }

    return this->frame_ready;
}

/*
void Frame_Mounter::mount_frame(Splitted_Frame input_frame)
{
    if (IDE == DOMINANT) {
        // base format
        this->frame = (CAN_Frame_Base*) malloc(sizeof(CAN_Frame_Base));
        CAN_Frame_Base* frame = (CAN_Frame_Base*) this->frame;
    
        frame->ID = input_frame.ID;
        
        // common to both formats
        frame->SOF = DOMINANT;
        frame->RTR = input_frame.RTR;
        frame->IDE = input_frame.IDE;
        frame->r0 = DOMINANT;
        frame->DLC = input_frame.PAYLOAD_SIZE;
        frame->Data = input_frame.PAYLOAD;
        frame->CRC = calculate_CRC();
        frame->CRC_delim = RECESSIVE;
        frame->ACK_slot = input_frame.ACK_slot;
        frame->ACK_delim = RECESSIVE;
        frame->EoF = B1111111;
    }
    else {
        // extended format
        this->frame = (CAN_Frame_Ext*) malloc(sizeof(CAN_Frame_Ext));
        CAN_Frame_Ext* frame = (CAN_Frame_Ext*) this->frame;

        frame->ID_A = ID >> 18;
        frame->ID_B = ID & 0x3FFFF; // 0x3FFFF = 18 bits 1
        frame->SSR = RECESSIVE;
        frame->r1 = DOMINANT;

        // common to both formats
        frame->SOF = DOMINANT;
        frame->RTR = input_frame.RTR;
        frame->IDE = input_frame.IDE;
        frame->r0 = DOMINANT;
        frame->DLC = input_frame.PAYLOAD_SIZE;
        frame->Data = input_frame.PAYLOAD;
        frame->CRC = calculate_CRC();
        frame->CRC_delim = RECESSIVE;
        frame->ACK_slot = input_frame.ACK_slot;
        frame->ACK_delim = RECESSIVE;
        frame->EoF = B1111111;
    }
}
*/

void Frame_Mounter::print_frame()
{
    if (((CAN_Frame_Base*) this->frame)->IDE == DOMINANT) {
        // base format
        CAN_Frame_Base* frame = (CAN_Frame_Base*) this->frame;
    
        Serial.print("SOF = ");
        Serial.println(frame->SOF, BIN);

        Serial.print("ID = ");
        Serial.print(frame->ID, DEC);
        Serial.print(" <=> ");
        Serial.println(frame->ID, BIN);

        Serial.print("RTR = ");
        Serial.println(frame->RTR, BIN);

        Serial.print("IDE = ");
        Serial.println(frame->IDE, BIN);

        Serial.print("r0 = ");
        Serial.println(frame->r0, BIN);

        Serial.print("DLC = ");
        Serial.print(frame->DLC, DEC);
        Serial.print(" <=> ");
        Serial.println(frame->DLC, BIN);

        Serial.print("Data = ");
        uint32_t data_low = frame->Data % 0xFFFFFFFF; 
        uint32_t data_high = (frame->Data >> 32) % 0xFFFFFFFF;
        print_uint64_t(frame->Data);
        Serial.print(" <=> ");
        Serial.print(data_low, BIN);
        Serial.println(data_high, BIN);

        Serial.print("CRC = ");
        Serial.print(frame->CRC, DEC);
        Serial.print(" <=> ");
        Serial.println(frame->CRC, BIN);

        Serial.print("CRC_delim = ");
        Serial.println(frame->CRC_delim, BIN);

        Serial.print("ACK_slot = ");
        Serial.println(frame->ACK_slot, BIN);

        Serial.print("ACK_delim = ");
        Serial.println(frame->ACK_delim, BIN);

        Serial.print("EOF = ");
        Serial.println(frame->EoF, BIN);
    }
    else {
        // extended format
        CAN_Frame_Ext* frame = (CAN_Frame_Ext*) this->frame;

        Serial.print("SOF = ");
        Serial.println(frame->SOF, BIN);

        Serial.print("ID = ");
        uint32_t id = ((uint32_t) frame->ID_A << 18) | frame->ID_B;
        Serial.print(id, DEC);
        Serial.print(" <=> ");
        Serial.print(" ( ");
        Serial.print(frame->ID_A, BIN);
        Serial.print(" ++ ");
        Serial.print(frame->ID_B, BIN);
        Serial.println(" )");

        Serial.print("SSR = ");
        Serial.println(frame->SSR, BIN);

        Serial.print("RTR = ");
        Serial.println(frame->RTR, BIN);

        Serial.print("IDE = ");
        Serial.println(frame->IDE, BIN);

        Serial.print("r1 = ");
        Serial.println(frame->r1, BIN);

        Serial.print("r0 = ");
        Serial.println(frame->r0, BIN);

        Serial.print("DLC = ");
        Serial.print(frame->DLC, DEC);
        Serial.print(" <=> ");
        Serial.println(frame->DLC, BIN);

        Serial.print("Data = ");
        uint32_t data_low = frame->Data % 0xFFFFFFFF; 
        uint32_t data_high = (frame->Data >> 32) % 0xFFFFFFFF;
        print_uint64_t(frame->Data);
        Serial.print(" <=> ");
        Serial.print(data_low, BIN);
        Serial.println(data_high, BIN);

        Serial.print("CRC = ");
        Serial.print(frame->CRC, DEC);
        Serial.print(" <=> ");
        Serial.println(frame->CRC, BIN);

        Serial.print("CRC_delim = ");
        Serial.println(frame->CRC_delim, BIN);

        Serial.print("ACK_slot = ");
        Serial.println(frame->ACK_slot, BIN);

        Serial.print("ACK_delim = ");
        Serial.println(frame->ACK_delim, BIN);
        
        Serial.print("EOF = ");
        Serial.println(frame->EoF, BIN);
    }
}