#ifndef FRAME_TRANSMITTER_H_INCLUDE
#define FRAME_TRANSMITTER_H_INCLUDE

#include <Arduino.h>
#include "datatypes/datatypes.h"

typedef enum frame_transmitter_states {
    INIT__Frame_Transmitter__,
    ACK__Frame_Transmitter__,
    ARBITRATION_PHASE_STD__Frame_Transmitter__,
    ARBITRATION_PHASE_EXT__Frame_Transmitter__,
    STANDARD__Frame_Transmitter__,
    EXTENDED__Frame_Transmitter__,
    BIT_ERROR__Frame_Transmitter__,
    SEND_ACTIVE_ERROR__Frame_Transmitter__,
    SEND_PASSIVE_ERROR__Frame_Transmitter__,
    BUS_OFF__Frame_Transmitter__
} Frame_Transmitter_States;

typedef struct frame_transmitter_input {
    Frame_Mounter_Output* frame_mounter;
    Bit_Stuffing_Reading_Output* bit_stuffing_rd;
    Error_Output* error;
} Frame_Transmitter_Input;

class Frame_Transmitter {
    private:
        Frame_Transmitter_Input input;
        Frame_Transmitter_Output* output;
        Frame_Transmitter_States state;
        uint8_t count;
        bool check_errors();
    public:
        Frame_Transmitter(Frame_Transmitter_Output &output);
        void setup(Frame_Mounter_Output &frame_mounter, Bit_Stuffing_Reading_Output &bit_stuffing_rd, Error_Output &error);
        void run();
};

#endif