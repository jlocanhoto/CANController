#ifndef SPLITTED_FRAME_H_INCLUDE
#define SPLITTED_FRAME_H_INCLUDE

typedef struct splitted_frame {
    uint32_t ID;
    bool IDE;
    bool RTR;
    uint64_t PAYLOAD;
    uint8_t PAYLOAD_SIZE;
} Splitted_Frame;

#endif