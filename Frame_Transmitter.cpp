#include "Frame_Transmitter.h"
#include "Error.h"

Frame_Transmitter::Frame_Transmitter(Frame_Transmitter_Output &output)
{
    output.lost_arbitration = LOW;
    this->output = &output;
    this->state = INIT__Frame_Transmitter__;
}

void Frame_Transmitter::setup(Frame_Mounter_Output &frame_mounter, Bit_Stuffing_Reading_Output &bit_stuffing_rd, Error_Output &error)
{
    this->input.frame_mounter = &frame_mounter;
    this->input.bit_stuffing_rd = &bit_stuffing_rd;
    this->input.error = &error;
}

void Frame_Transmitter::errors_check()
{
    if (this->input.error->error_detected == HIGH) { //CONTROLE DE ERRO
        if (this->input.error->error_state == BUSS_OFF_CODE) { //Código de erro Bus_Off
            this->state = BUS_OFF__Frame_Transmitter__;
        } 
        else {
            this->count = 0;
            this->output->stuffing_enable = LOW;

            if (this->input.error->error_state == ACTIVE_ERROR_CODE) { //Código de erro ativo
                this->state = SEND_ACTIVE_ERROR__Frame_Transmitter__;
            }
            else if (this->input.error->error_state == PASSIVE_ERROR_CODE) { //Código de erro passivo
                this->state = SEND_PASSIVE_ERROR__Frame_Transmitter__;
            }            
        }
    }

    return this->input.error->error_detected;
}

void Frame_Transmitter::run()
{
    switch (this->state)
    {
        case INIT__Frame_Transmitter__:
        {
            this->count = 0;
            this->output->arb_output = HIGH;
            this->output->stuffing_enable = LOW;

            if (!this->check_errors()) {
                if(ack == 0 && ARB_wp == 1) {
                    this->state = ACK__Frame_Transmitter__;
                }
                else if(frame_ready == 1 && ARB_wp == 1) {
                    this->output->stuffing_enable = HIGH;
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                    this->output->lost_arbitration = LOW;

                    this->output->eof = LOW;

                    if(IDE == 0) {
                        this->state = ARBITRATION_PHASE_STD__Frame_Transmitter__;
                    }
                    else {
                        this->state = ARBITRATION_PHASE_EXT__Frame_Transmitter__;
                    }        
                }
            }
            
            break;
        }
        case ACK__Frame_Transmitter__:
        {
            this->output->arb_output = ack;

            if(ARB_wp == 1) {
                this->state = INIT__Frame_Transmitter__;
            }
            break;
        }
        case ARBITRATION_PHASE_STD__Frame_Transmitter__:
        {
            if(error_detected == 1) { //CONTROLE DE ERRO
                if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                    this->state = BUS_OFF;
                } 
                else {
                    this->count = 0;
                    stuffing_enable = 0;
                    if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                        this->state = SEND_ACTIVE_ERROR;
                    }
                    else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                        this->state = SEND_PASSIVE_ERROR;
                    }            
                }
            }
            else {
                if (ARB_wp == 1) {
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                }
                
                if(new_sp == 1 && new_input != this->output->arb_output) {
                    lost_arbitration = 1;
                    this->state = INICIO;
                    break;
                }
                if(this->count == 13) {
                    for (int i = 15; i <= 18; i++) {
                        dlc[i-15] = FRAME[i];
                    }
                    data_limit = 18 + 8*int(dlc); //ver a função pra transformar o dlc pra inteiro em C++
                    this->state = STANDARD;
                    break;
                }
            }
            
            break;
        }
        case ARBITRATION_PHASE_EXT__Frame_Transmitter__: 
        {
            if(error_detected == 1) { //CONTROLE DE ERRO
                if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                    this->state == BUS_OFF;
                } 
                else {
                    this->count = 0;
                    stuffing_enable = 0;
                    if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                        this->state = SEND_ACTIVE_ERROR;
                    }
                    else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                        this->state = SEND_PASSIVE_ERROR;
                    }            
                }            
            }
            else {
                if(ARB_wp == 1) {
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                }
                
                if(new_sp == 1 && new_input != this->output->arb_output) {
                    lost_arbitration = 1;
                    this->state = INICIO;
                    break;
                }
                
                if(this->count == 33) {
                    for (int i = 35; i <= 38; i++) {
                        dlc[i-35] = FRAME[i];
                    }
                    data_limit = 38 + 8*int(dlc); //ver a função pra transformar o dlc pra inteiro em C++
                    this->state = EXTENDED;
                    break;
                }
            }        
            break;
        }
        case STANDARD__Frame_Transmitter__:
        {
            if(error_detected == 1) { //CONTROLE DE ERRO
                if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                    this->state == BUS_OFF;
                } 
                else {
                    this->count = 0;
                    stuffing_enable = 0;
                    if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                        this->state = SEND_ACTIVE_ERROR;
                    }
                    else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                        this->state = SEND_PASSIVE_ERROR;
                    }            
                }            
            }
            else {
                if(ARB_wp == 1) {
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                }

                if(new_sp == 1 && new_input != this->output->arb_output && this->count != data_limit + 16) { //this->count != ack slot
                    this->state = BIT_ERROR;
                    break;
                }

                if(this->count == data_limit + 16) {
                    stuffing_enable = 0;
                }

                if(this->count == data_limit + 29) { //this->count != ack slot
                    eof = 1;
                    this->state = INICIO;
                }          
            }        
            break;
        }
        case EXTENDED__Frame_Transmitter__:
        {
            if(error_detected == 1) { //CONTROLE DE ERRO
                if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                    this->state == BUS_OFF;
                } 
                else {
                    this->count = 0;
                    stuffing_enable = 0;
                    if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                        this->state = SEND_ACTIVE_ERROR;
                    }
                    else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                        this->state = SEND_PASSIVE_ERROR;
                    }            
                }            
            }
            else {
                if(ARB_wp == 1) {
                    this->output->arb_output = FRAME[this->count];
                    this->count++;
                }

                if(new_sp == 1 && new_input != this->output->arb_output && this->count != data_limit + 16) { //this->count != ack slot
                    this->state = BIT_ERROR;
                    break;
                }

                if(this->count == data_limit + 16) {
                    stuffing_enable = 0;
                }

                if(this->count == data_limit + 29) { //this->count != ack slot
                    eof = 1;
                    this->state = INICIO;
                }
            }
            break;
        }
        case BIT_ERROR__Frame_Transmitter__:
        {
            bit_error = 1;
            if(error_detected == 1) { //CONTROLE DE ERRO
                if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                    this->state == BUS_OFF;
                } 
                else {
                    this->count = 0;
                    stuffing_enable = 0;
                    if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                        this->state = SEND_ACTIVE_ERROR;
                    }
                    else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                        this->state = SEND_PASSIVE_ERROR;
                    }            
                }            
            }
            break;
        }
        case SEND_ACTIVE_ERROR__Frame_Transmitter__:
        {
            this->output->arb_output = 0;

            if(this->count < 6 && ARB_wp == 1) {
                this->count++;
            }
            else if(this->count == 6 && ARB_wp == 1) {
                this->output->arb_output = 1;
                this->state = INICIO;
            }
            break;
        }
        case SEND_PASSIVE_ERROR__Frame_Transmitter__:
        {
            this->output->arb_output = 1;

            if(this->count < 6 && ARB_wp == 1) {
                this->count++;
            }
            else if(this->count == 6 && ARB_wp == 1) {
                this->output->arb_output = 1;
                this->state = INICIO;
            }
            break;
        }
        case BUS_OFF__Frame_Transmitter__:
        {
            printf("WAIT SYSTEM RESET!!");
            break;
        }
    }
}