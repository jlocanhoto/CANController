#ifndef ERROR_H_INCLUDE
#define ERROR_H_INCLUDE

#define BUS_OFF_CODE        0
#define ACTIVE_ERROR_CODE   1
#define PASSIVE_ERROR_CODE  2

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
    public:
        Error(Error_Data &output);
        void connect_inputs(Bit_Stuffing_Reading_Data &Bit_Stuff_Read, Decoder_Data &decoder, Frame_Transmitter_Data &Frame_Transmitter);
        void run();
}
#endif