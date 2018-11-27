#ifndef FRAME_TRANSMITTER_DATATYPE_H_INCLUDE
#define FRAME_TRANSMITTER_DATATYPE_H_INCLUDE

typedef struct frame_transmitter_output {
    bool lost_arbitration;
    bool ack_error;
    bool bit_error;
    bool stuffing_enable;
    bool EoF;
    bool arb_output;
} Frame_Transmitter_Output;

#endif