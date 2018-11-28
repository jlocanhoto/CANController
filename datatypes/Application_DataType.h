#ifndef APPLICATION_DATATYPE_H_INCLUDE
#define APPLICATION_DATATYPE_H_INCLUDE

#include "Splitted_Frame.h"

typedef struct application_output {
    bool new_frame;
    Splitted_Frame output_frame;
    bool ACK_slot;
} Application_Data;

#endif