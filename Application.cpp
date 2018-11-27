#include <Arduino.h>
#include "Application.h"
#include "config.h"
#include "utils.h"

void random_frame(Splitted_Frame &frame, bool &new_frame)
{
    if (new_frame == LOW) {
        frame.ID = 0x48D;
        frame.IDE = DOMINANT;
        frame.RTR = DOMINANT;
        frame.PAYLOAD = 0x01;
        frame.PAYLOAD_SIZE = 0x01;
        frame.ACK_slot = RECESSIVE;

        new_frame = HIGH;

        Serial.println("[INPUT FRAME]");

        Serial.print("ID = ");
        Serial.println(frame.ID, BIN);

        Serial.print("IDE = ");
        Serial.println(frame.IDE, DEC);

        Serial.print("RTR = ");
        Serial.println(frame.RTR, DEC);

        Serial.print("PAYLOAD = ");
        print_uint64_t(frame.PAYLOAD);
        Serial.println();

        Serial.print("PAYLOAD_SIZE = ");
        Serial.println(frame.PAYLOAD_SIZE, DEC);

        Serial.print("ACK_slot = ");
        Serial.println(frame.ACK_slot, DEC);
    }
}