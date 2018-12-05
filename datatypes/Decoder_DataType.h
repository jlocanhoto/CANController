#ifndef DECODER_DATATYPE_H_INCLUDE
#define DECODER_DATATYPE_H_INCLUDE

#include "Splitted_Frame.h"

typedef struct decoder_output {
    // to Frame Transmitter
    bool ack;
    bool ack_slot;
    // to Bit Stuffing on Reading
    bool stuffing_enable;
    // to CRC Calculator
    bool* PARTIAL_FRAME;
    // to Error Treatment
    bool crc_error;
    bool format_error;
    // to BTL + Application
    bool EoF;
    // to Application
    Splitted_Frame decoded_frame;
} Decoder_Data;

#endif