#ifndef BIT_STUFFING_READING_H_INCLUDE
#define BIT_STUFFING_READING_H_INCLUDE

#include <Arduino.h>
#include "config.h"
#include "datatypes/datatypes.h"

typedef enum bit_stuffing_reading_states {
    INIT__Bit_Stuffing_Reading__,
    STUFF__Bit_Stuffing_Reading__,
    ERROR__Bit_Stuffing_Reading__
} Bit_Stuffing_Reading_States;

typedef struct bit_stuffing_reading_input {
    BTL_Data* BTL;
    Decoder_Data* decoder;
} Bit_Stuffing_Reading_Input;

class Bit_Stuffing_Reading {
    private: 
        Bit_Stuffing_Reading_Input input;
        Bit_Stuffing_Reading_Data* output;
        Bit_Stuffing_Reading_States state;
        uint8_t count;
        bool previous_bit;
        bool previous_sp_pt;
    public:
        Bit_Stuffing_Reading(Bit_Stuffing_Reading_Data &output);
        void connect_inputs(Decoder_Data &decoder, BTL_Data &BTL);
        void run();
};

#endif