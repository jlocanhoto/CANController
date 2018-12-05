#ifndef ERROR_H_INCLUDE
#define ERROR_H_INCLUDE

#define BUS_OFF_CODE        0
#define ACTIVE_ERROR_CODE   1
#define PASSIVE_ERROR_CODE  2

#define ACTIVE_TO_PASSIVE_LIMIT     127
#define PASSIVE_TO_BUS_OFF_LIMIT    255

#define TRANSMIT_ERROR_POINTS   8
#define RECEIVE_ERROR_POINTS    1

#include <Arduino.h>
#include "datatypes/datatypes.h"

typedef enum error_states {
    INIT__error_treatment__,
    ERROR__error_treatment__
} Error_States;

typedef struct error_treatment_input {
    Bit_Stuffing_Reading_Data* Bit_Stuff_Read;
    Decoder_Data* decoder;
    Frame_Transmitter_Data* Frame_Transmitter;
} Error_Input;

class Error {
    private:
        Error_Input input;
        Error_Data* output;
        Error_States state;
        uint8_t count;
        uint16_t TEC; // Transmit Error Counter
        uint8_t REC; // Receive Error Counter
        bool prev_stuff_error;
        bool prev_crc_error;
        bool prev_format_error;
        bool prev_bit_error;
        bool prev_ack_error;
        bool prev_ack_ok_rx;
        bool prev_ack_ok_tx;
        bool prev_bus_on;
    public:
        Error(Error_Data &output);
        void connect_inputs(Bit_Stuffing_Reading_Data &Bit_Stuff_Read, Decoder_Data &decoder, Frame_Transmitter_Data &Frame_Transmitter);
        void run();
};

#endif