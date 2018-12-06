#include "CRC_Calculator.h"

uint16_t can_crc_next(uint16_t crc, uint8_t data)
{
    uint8_t i, j;

    crc ^= (uint16_t)data << 7;

    for (i = 0; i < 8; i++) {
        crc <<= 1;
        if (crc & 0x8000) {
            crc ^= 0xc599;
        }
    }

    return crc & 0x7fff;
}

void calculate_CRC(CRC_Data &crc_data, bool* PARTIAL_FRAME)
{
    uint8_t offset = 0;

    if (crc_data.crc_req) {
        offset = CRC_SIZE;
    }
    do {
        if (crc_data.bit_counter < crc_data.PT_COUNTER + offset)
        {
            crc_data.CRC <<= 1;
            
            if (crc_data.bit_counter < crc_data.PT_COUNTER) {
                crc_data.CRC |= PARTIAL_FRAME[crc_data.bit_counter];
            }
            
            if (crc_data.CRC & ORDER_BIT) {
                crc_data.CRC ^= POLYNOMIAL;
            }

            crc_data.CRC &= 0x7FFF;
            crc_data.bit_counter++;
        }
        else if (crc_data.crc_req) {
            crc_data.crc_ready = HIGH;
            //Serial.print("Final CRC = ");
            //Serial.println(crc_data.CRC, BIN);
            //delay(2000);
        }
    } while (crc_data.crc_ready == LOW && crc_data.crc_req == HIGH);
}

void reset_CRC(CRC_Data* crc_data)
{
    crc_data->CRC = 0;
    crc_data->PT_COUNTER = 0;
    crc_data->bit_counter = 0;
    crc_data->crc_ready = LOW;
    crc_data->crc_req = LOW;
}