#include "Error.h"

Error::Error(Error_Data &output)
{
    output.error_detected = LOW;
    output.error_state = ACTIVE_ERROR_CODE;
    this->count = 0;
    this->TEC = 0;
    this->REC = 0;
    this->output = &output;
    this->state = INIT__error_treatment__;
}

void Error::connect_inputs(Bit_Stuffing_Reading_Data &Bit_Stuff_Read, Decoder_Data &decoder, Frame_Transmitter_Data &Frame_Transmitter)
{
    this->input.Bit_Stuff_Read = &Bit_Stuff_Read;
    this->input.decoder = &decoder;
    this->input.Frame_Transmitter = &Frame_Transmitter;
}

void Error::run()
{
    bool stuff_error_edge = false;
    bool crc_error_edge = false;
    bool format_error_edge = false;
    bool bit_error_edge = false;
    bool ack_error_edge = false;
    bool ack_ok_rx_edge = false;
    bool ack_ok_tx_edge = false;
    bool bus_on_edge = false;

    // this comparison is done before the counters change because when node becomes error passive, it still will send an ACTIVE ERROR FLAG in this transition
    if (this->TEC > 127 || this->REC > 127) {
        this->output->error_state = PASSIVE_ERROR_CODE;
    }
    else if (this->TEC > 255) {
        this->output->error_state = BUS_OFF_CODE;
    }

    if (this->prev_stuff_error == LOW && this->input.Bit_Stuff_Read->stuff_error == HIGH) {
        stuff_error_edge = true;
    }
    if (this->prev_crc_error == LOW && this->input.decoder->crc_error == HIGH) {
        crc_error_edge = true;
    }
    if (this->prev_format_error == LOW && this->input.decoder->format_error == HIGH) {
        format_error_edge = true;
    }
    if (this->prev_bit_error == LOW && this->input.Frame_Transmitter->bit_error == HIGH) {
        bit_error_edge = true;
    }
    if (this->prev_ack_error == LOW && this->input.Frame_Transmitter->ack_error == HIGH) {
        ack_error_edge = true;
    }
    if (this->prev_ack_ok_rx == LOW && this->input.Frame_Transmitter->ack_ok_rx == HIGH) {
        ack_ok_rx_edge = true;
    }
    if (this->prev_ack_ok_tx == LOW && this->input.Frame_Transmitter->ack_ok_tx == HIGH) {
        ack_ok_tx_edge = true;
    }
    if (this->prev_bus_on == LOW && this->input.Frame_Transmitter->bus_on == HIGH) {
        bus_on_edge = true;
    }

    if (bus_on_edge) {
        this->TEC = this->REC = 0;
    }

    if (bit_error_edge | ack_error_edge) {
        Serial.println("TEC + 8");
        this->TEC += TRANSMIT_ERROR_POINTS;
    }
    // missing when REC += 8
    if (stuff_error_edge | crc_error_edge | format_error_edge) {
        Serial.println("REC + 1");
        this->REC += RECEIVE_ERROR_POINTS;
    }
    if (ack_ok_rx_edge && this->REC > 0) {
        if (this->REC > 127) {
            this->REC -= 8;
            Serial.println("REC - 8");
        }
        else {
            Serial.println("REC - 1");
            this->REC--;
        }
    }
    if (ack_ok_tx_edge && this->TEC > 0) {
        Serial.println("TEC - 1");
        this->TEC--;
    }

    this->output->error_detected = stuff_error_edge | crc_error_edge | format_error_edge | bit_error_edge | ack_error_edge;

    if (this->output->error_detected) {
        
    }

    this->prev_stuff_error = this->input.Bit_Stuff_Read->stuff_error;
    this->prev_crc_error = this->input.decoder->crc_error;
    this->prev_format_error = this->input.decoder->format_error;
    this->prev_bit_error = this->input.Frame_Transmitter->bit_error;
    this->prev_ack_error = this->input.Frame_Transmitter->ack_error;
    this->prev_ack_ok_rx = this->input.Frame_Transmitter->ack_ok_rx;
    this->prev_ack_ok_tx = this->input.Frame_Transmitter->ack_ok_tx;
    this->prev_bus_on = this->input.Frame_Transmitter->bus_on;
}