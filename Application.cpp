#include "Application.h"

void random_frame(Application_Data &output)
{
    output.output_frame.ID = 0x48D;
    output.output_frame.IDE = DOMINANT;
    output.output_frame.RTR = DOMINANT;
    output.output_frame.PAYLOAD = 0x01;
    output.output_frame.PAYLOAD_SIZE = 0x01;
    output.ACK_slot = RECESSIVE;

    output.new_frame = HIGH;

    Serial.println("[INPUT FRAME]");

    Serial.print("ID = ");
    Serial.println(output.output_frame.ID, BIN);

    Serial.print("IDE = ");
    Serial.println(output.output_frame.IDE, DEC);

    Serial.print("RTR = ");
    Serial.println(output.output_frame.RTR, DEC);

    Serial.print("PAYLOAD = ");
    print_uint64_t(output.output_frame.PAYLOAD);
    Serial.println();

    Serial.print("PAYLOAD_SIZE = ");
    Serial.println(output.output_frame.PAYLOAD_SIZE, DEC);

    Serial.print("ACK_slot = ");
    Serial.println(output.ACK_slot, DEC);
}