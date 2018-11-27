#ifndef FRAME_MOUNTER_DATATYPE_H_INCLUDE
#define FRAME_MOUNTER_DATATYPE_H_INCLUDE

typedef struct Frame_Mounter_Data {
    bool* FRAME;
    bool frame_ready;
    uint8_t data_limit;
    uint8_t arb_limit;
} Frame_Mounter_Data;

#endif