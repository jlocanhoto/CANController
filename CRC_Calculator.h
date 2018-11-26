#ifndef CRC_CALCULATOR_H_INCLUDE
#define CRC_CALCULATOR_H_INCLUDE

#include <Arduino.h>

#define POLYNOMIAL  0x4599 // (x^15+x^14+x^10+x^8+x^7+x^4+x^3+1)
#define ORDER_BIT   0x8000
#define CRC_SIZE    15

typedef struct CRC_data {
    uint16_t CRC;
    uint8_t PT_COUNTER;
    uint8_t bit_counter;
    bool crc_ready;
    bool crc_req;
} CRC_Data;

void calculate_CRC(CRC_Data &crc_data, bool* PARTIAL_FR);
void reset_CRC(CRC_Data &crc_data);
uint16_t can_crc_next(uint16_t crc, uint8_t data);

#endif