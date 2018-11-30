#ifndef DECODER_H_INCLUDE
#define DECODER_H_INCLUDE

#include <Arduino.h>
#include "config.h"
#include "datatypes/datatypes.h"
#include "CRC_Calculator.h"
#include "frame_positions.h"
#include "utils.h"

typedef enum decoder_states {
    INIT_SOF__Decoder__,
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
    CRC_Data* crc_interface;
    Frame_Transmitter_Data* frame_transmitter;
} Decoder_Input;

class Decoder {
    private:
        Decoder_Input input;
        Decoder_Data* output;
        Decoder_States state;
        uint8_t count;
        bool arb;
        bool crc_ok;
        bool previous_sample_pt;
        bool EOF_signal;

        bool SOF;
        bool r0;
        bool r1;
        uint16_t CRC;
        bool SRR;
        bool CRC_delim;
        bool ACK_slot;
        bool ACK_delim;
        uint8_t EoF;

        bool previous_EoF_frame_mounter;
    public:
        Decoder(Decoder_Data &output, uint16_t partial_frame_size);
        void connect_inputs(Bit_Stuffing_Reading_Data &bit_stuffing_rd, CRC_Data &crc_interface, Frame_Transmitter_Data &frame_transmitter);
        void run();
        void print_frame();
};

#endif