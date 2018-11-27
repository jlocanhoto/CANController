#ifndef BIT_STUFFING_READING_DATATYPE_H_INCLUDE
#define BIT_STUFFING_READING_DATATYPE_H_INCLUDE

typedef struct bit_stuffing_reading_output {
    bool new_sampled_bit;
    bool new_sample_pt;
    bool stuff_error;
} Bit_Stuffing_Reading_Data;

#endif