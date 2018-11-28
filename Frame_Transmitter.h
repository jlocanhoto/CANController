#ifndef FRAME_TRANSMITTER_H_INCLUDE
#define FRAME_TRANSMITTER_H_INCLUDE

#include <Arduino.h>
#include "datatypes/datatypes.h"
#include "frame_positions.h"

typedef enum frame_transmitter_states {
    INIT__Frame_Transmitter__,
    ACK__Frame_Transmitter__,
    ARBITRATION_PHASE__Frame_Transmitter__,
    DATA_TO_END_PHASE__Frame_Transmitter__,
    BIT_ERROR__Frame_Transmitter__,
    SEND_ACTIVE_ERROR__Frame_Transmitter__,
    SEND_PASSIVE_ERROR__Frame_Transmitter__,
    BUS_OFF__Frame_Transmitter__
} Frame_Transmitter_States;

typedef struct frame_transmitter_input {
    Frame_Mounter_Data* frame_mounter;
    Decoder_Data* decoder;
    Bit_Stuffing_Writing_Data* bit_stuffing_wr;
    Bit_Stuffing_Reading_Data* bit_stuffing_rd;    
    Error_Data* error;
} Frame_Transmitter_Input;

class Frame_Transmitter {
    private:
        Frame_Transmitter_Input input;
        Frame_Transmitter_Data* output;
        Frame_Transmitter_States state;
        uint8_t count;
        bool check_errors();
    public:
        Frame_Transmitter(Frame_Transmitter_Data &output);
        void connect_inputs(Frame_Mounter_Data &frame_mounter, Bit_Stuffing_Writing_Data &bit_stuffing_wr, Bit_Stuffing_Reading_Data &bit_stuffing_rd, Error_Data &error, Decoder_Data &decoder);
        void run();
};

#endif