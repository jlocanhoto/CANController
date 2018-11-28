#ifndef BIT_STUFFING_WRITING_H_INCLUDE
#define BIT_STUFFING_WRITING_H_INCLUDE

#include <Arduino.h>
#include "config.h"
#include "datatypes/datatypes.h"

typedef enum bit_stuffing_writing_states {
    INIT__Bit_Stuffing_Writing__,
    STUFF__Bit_Stuffing_Writing__
} Bit_Stuffing_Writing_States;

typedef struct bit_stuffing_writing_input {
    BTL_Data* BTL;
    Frame_Transmitter_Data* frame_transmitter;
} Bit_Stuffing_Writing_Input;

class Bit_Stuffing_Writing {
    private:
        Bit_Stuffing_Writing_Input input;
        Bit_Stuffing_Writing_Data* output;
        Bit_Stuffing_Writing_States state;
        uint8_t count;
        bool previous_bit;
        bool previous_wr_pt;
    public:
        Bit_Stuffing_Writing(Bit_Stuffing_Writing_Data &output);
        void connect_inputs(Frame_Transmitter_Data &frame_transmitter, BTL_Data &BTL);
        void run();
};

#endif