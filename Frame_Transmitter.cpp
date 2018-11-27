#include "Frame_Transmitter.h"

Frame_Transmitter::Frame_Transmitter(Frame_Transmitter_Output &output)
{
    output.lost_arbitration = LOW;
    this->output = &output;
}

void Frame_Transmitter::interconnect(Frame_Mounter_Output &frame_mounter, Bit_Stuffing_Reading_Output &bit_stuffing_rd, Error_Output &error)
{
    this->input.frame_mounter = &frame_mounter;
    this->input.bit_stuffing_rd = &bit_stuffing_rd;
    this->input.error = &error;
}

void Frame_Transmitter::run()
{
    static Frame_Transmitter_States state = INIT__Frame_Transmitter__;
}