#ifndef CRC_CALCULATOR_H_INCLUDE
#define CRC_CALCULATOR_H_INCLUDE

#include <Arduino.h>
#include "datatypes/datatypes.h"

#define POLYNOMIAL  0x4599 // (x^15+x^14+x^10+x^8+x^7+x^4+x^3+1)
#define ORDER_BIT   0x8000
#define CRC_SIZE    15

void calculate_CRC(CRC_Data &crc_data, bool* PARTIAL_FR);
void reset_CRC(CRC_Data* crc_data);
uint16_t can_crc_next(uint16_t crc, uint8_t data);

#endif