#include "Application.h"

void custom_frame(Application_Data &output)
{
    output.output_frame.ID = 0x411;
    output.output_frame.IDE = DOMINANT;
    output.output_frame.RTR = DOMINANT;
    output.output_frame.PAYLOAD = 0xABCDEF;
    output.output_frame.PAYLOAD_SIZE = 0x0;
    output.ACK_slot = DOMINANT;

    output.new_frame = HIGH;

    Serial.println("[OUTPUT FRAME]");

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