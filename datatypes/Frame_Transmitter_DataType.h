#ifndef FRAME_TRANSMITTER_DATATYPE_H_INCLUDE
#define FRAME_TRANSMITTER_DATATYPE_H_INCLUDE

typedef struct frame_transmitter_output {
    bool lost_arbitration;
    bool ack_error;
    bool bit_error;
    bool stuffing_enable;
    bool arb_output;
    bool eof;
} Frame_Transmitter_Data;

#endif