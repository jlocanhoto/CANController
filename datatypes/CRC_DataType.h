#ifndef CRC_DATATYPE_H_INCLUDE
#define CRC_DATATYPE_H_INCLUDE

typedef struct CRC_data {
    uint16_t CRC;
    uint8_t PT_COUNTER;
    uint8_t bit_counter;
    bool crc_ready;
    bool crc_req;
} CRC_Data;

#endif