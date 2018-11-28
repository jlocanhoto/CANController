#ifndef DECODER_DATATYPE_H_INCLUDE
#define DECODER_DATATYPE_H_INCLUDE

#include "Splitted_Frame.h"

typedef struct decoder_output {
    // to Frame Transmitter
    bool ack;
    // to Bit Stuffing on Reading
    bool stuffing_enable;
    // to CRC Calculator
    bool crc_req;
    bool* PARTIAL_FR;
    uint8_t PT_COUNTER;
    // to Error Treatment
    bool crc_error;
    bool format_error;
    // to BTL + Application
    bool EoF;
    // to Application
    Splitted_Frame decoded_frame;
} Decoder_Data;

#endif