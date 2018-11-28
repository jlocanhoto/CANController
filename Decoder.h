#ifndef BIT_STUFFING_WRITING_H_INCLUDE
#define BIT_STUFFING_WRITING_H_INCLUDE

#include <Arduino.h>
#include "config.h"
#include "datatypes/datatypes.h"

typedef enum decoder_states {
    INIT__Decoder__,
    SOF__Decoder__,
    IDENTIFIER_A__Decoder__,
    RTR_SRR__Decoder__,
    IDE__Decoder__,
    IDENTIFIER_B__Decoder__,
    RTR_EXT__Decoder__,
    r1__Decoder__,
    r0__Decoder__,
    STAND_BY__Decoder__,
    DLC__Decoder__,
    DATA_FIELD__Decoder__,
    CRC__Decoder__,
    CRC_delimiter__Decoder__,
    ACK_slot__Decoder__,
    ACK_delimiter__Decoder__,
    EOF__Decoder__,
    ERROR__Decoder__
} Decoder_States;

typedef struct decoder_input {
    Bit_Stuffing_Reading_Data* bit_stuffing_rd;
    CRC_Calculator_Data* crc_calc;
    Frame_Transmitter_Data* frame_transmitter;
} Decoder_Input;

class Bit_Stuffing_Writing {
    private:
        Decoder_Input input;
        Decoder_Data* output;
        Decoder_States state;
        uint8_t count;
        bool arb;
    public:
        Bit_Stuffing_Writing(Decoder_Data &output, uint16_t partial_frame_size);
        void connect_inputs(Bit_Stuffing_Reading_Data &bit_stuffing_rd, CRC_Calculator_Data &crc_calc, Frame_Transmitter_Data &frame_transmitter);
        void run();
};

#endif