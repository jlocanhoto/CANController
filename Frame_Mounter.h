#ifndef FRAME_MOUNTER_H_INCLUDE
#define FRAME_MOUNTER_H_INCLUDE

#include <Arduino.h>
#include "config.h"
#include "Application.h"
#include "CRC_Calculator.h"
#include "Frame_Transmitter.h"
#include "datatypes/datatypes.h"
#include "frame_positions.h"

enum frame_formats {
    base_format = DOMINANT,
    extended_format = RECESSIVE
};

enum frame_types {
    data_frame = DOMINANT,
    remote_frame = RECESSIVE,
    error_frame,
    overload_frame
};

typedef enum frame_mounter_states {
    INIT__Frame_Mounter__,
    BASE_FORMAT__Frame_Mounter__,
    EXTENDED_FORMAT__Frame_Mounter__,
    DATA_FIELD__Frame_Mounter__,
    CRC_WAIT__Frame_Mounter__,
    CRC_ACK_EOF__Frame_Mounter__
} Frame_Mounter_States;

typedef struct can_frame_base {
    uint8_t SOF : 1;
    uint16_t ID : 11;
    uint8_t RTR : 1;
    uint8_t IDE : 1;
    uint8_t r0 : 1;
    uint8_t DLC : 4;
    uint64_t Data: MAX_PAYLOAD_SIZE;
    uint16_t CRC : 15;
    uint8_t CRC_delim : 1;
    uint8_t ACK_slot : 1;
    uint8_t ACK_delim : 1;
    uint8_t EoF : 7;
} CAN_Frame_Base;

typedef struct can_frame_ext {
    uint8_t SOF : 1;
    uint16_t ID_A : 11;
    uint8_t SSR : 1;
    uint8_t IDE : 1;
    uint32_t ID_B : 18;
    uint8_t RTR : 1;
    uint8_t r1 : 1;
    uint8_t r0 : 1;
    uint8_t DLC : 4;
    uint64_t Data: MAX_PAYLOAD_SIZE;
    uint16_t CRC : 15;
    uint8_t CRC_delim : 1;
    uint8_t ACK_slot : 1;
    uint8_t ACK_delim : 1;
    uint8_t EoF : 7;
} CAN_Frame_Ext;

typedef struct frame_mounter_input {
    bool new_frame;
    Splitted_Frame input_frame;
    CRC_Data crc_data;
} Frame_Mounter_Input;

class Frame_Mounter {
    private:
        void* frame;
        bool frame_ready;
        Frame_Mounter_Input input;
        Frame_Mounter_Data* output;
        Frame_Mounter_States state;
    public:
        Frame_Mounter();
        //Frame_Mounter(Frame_Mounter &output);
        //void connect_inputs(Frame_Mounter_Data &frame_mounter, Bit_Stuffing_Writing_Data &bit_stuffing_wr, Error_Data &error, Decoder_Data &decoder);
        //void run();
        bool mount(bool new_frame, Splitted_Frame &input_frame, bool ACK_slot, CRC_Data &crc_data, bool* FRAME);
        void print_frame();
};

#endif