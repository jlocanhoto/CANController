#ifndef APPLICATION_H_INCLUDE
#define APPLICATION_H_INCLUDE

typedef struct splitted_frame {
    uint32_t ID;
    bool IDE;
    bool RTR;
    uint64_t PAYLOAD;
    uint8_t PAYLOAD_SIZE;
    bool ACK_slot;
} Splitted_Frame;

void random_frame(Splitted_Frame &frame, bool &new_frame);

#endif