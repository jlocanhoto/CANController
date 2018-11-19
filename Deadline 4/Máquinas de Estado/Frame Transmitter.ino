#include <stdio.h>
#define INICIO '0'
#define ACK '1'
#define ARBITRATION_PHASE_STD '2'
#define ARBITRATION_PHASE_EXT '3'
#define STANDARD '4'
#define EXTENDED '5'
#define BIT_ERROR '6'
#define SEND_ACTIVE_ERROR '7'
#define SEND_PASSIVE_ERROR '8'
#define BUS_OFF '9'

int count, start_data, data_limit, dlc[4], bit_error;
int ack, ARB_wp, frame_ready, IDE, FRAME[128], error_detected, error_state, new_sp, new_input; //INPUTS
int lost_arbitration, ARB_output, stuffing_enable, eof;// OUTPUTS
char controleEstado;

void setup() {
    controleEstado = INICIO;  
    lost_arbitration = 0;
}

void loop() {
  switch(controleEstado){
    case INICIO:
    {
        count = 0;
        ARB_output = 1;
        stuffing_enable = 0;

        if(error_detected == 1) { //CONTROLE DE ERRO
            if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                controleEstado == BUS_OFF;
            } 
            else {
                count = 0;
                stuffing_enable = 0;
                if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                    controleEstado = SEND_ACTIVE_ERROR;
                }
                else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                    controleEstado = SEND_PASSIVE_ERROR;
                }            
            }
        }
        else {
            if(ack == 0 && ARB_wp == 1) {
                controleEstado = ACK;
            }
            else if(frame_ready == 1 && ARB_wp == 1) {
                stuffing_enable = 1;
                ARB_output = FRAME[count];
                count++;
                lost_arbitration = 0;
                eof = 0;
                if(IDE == 0) {
                    controleEstado = ARBITRATION_PHASE_STD;
                }
                else {
                    controleEstado = ARBITRATION_PHASE_EXT;
                }        
            }
        }
        
        break;
    }
    case ACK:
    {
        ARB_output = ack;

        if(ARB_wp == 1) {
            controleEstado = INICIO;
        }
        break;
    }
    case ARBITRATION_PHASE_STD:
    {
        if(error_detected == 1) { //CONTROLE DE ERRO
            if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                controleEstado == BUS_OFF;
            } 
            else {
                count = 0;
                stuffing_enable = 0;
                if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                    controleEstado = SEND_ACTIVE_ERROR;
                }
                else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                    controleEstado = SEND_PASSIVE_ERROR;
                }            
            }
        }
        else {
            if (ARB_wp == 1) {
                ARB_output = FRAME[count];
                count++;
            }
            
            if(new_sp == 1 && new_input != ARB_output) {
                lost_arbitration = 1;
                controleEstado = INICIO;
                break;
            }
            if(count == 13) {
                for (int i = 15; i <= 18; i++) {
                    dlc[i-15] = FRAME[i];
                }
                data_limit = 18 + 8*int(dlc); //ver a função pra transformar o dlc pra inteiro em C++
                controleEstado = STANDARD;
                break;
            }
        }       
        break;
    }
    case ARBITRATION_PHASE_EXT: 
    {
        if(error_detected == 1) { //CONTROLE DE ERRO
            if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                controleEstado == BUS_OFF;
            } 
            else {
                count = 0;
                stuffing_enable = 0;
                if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                    controleEstado = SEND_ACTIVE_ERROR;
                }
                else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                    controleEstado = SEND_PASSIVE_ERROR;
                }            
            }            
        }
        else {
            if(ARB_wp == 1) {
                ARB_output = FRAME[count];
                count++;
            }
            
            if(new_sp == 1 && new_input != ARB_output) {
                lost_arbitration = 1;
                controleEstado = INICIO;
                break;
            }
            
            if(count == 33) {
                for (int i = 35; i <= 38; i++) {
                    dlc[i-35] = FRAME[i];
                }
                data_limit = 38 + 8*int(dlc); //ver a função pra transformar o dlc pra inteiro em C++
                controleEstado = EXTENDED;
                break;
            }
        }        
        break;
    }
    case STANDARD:
    {
        if(error_detected == 1) { //CONTROLE DE ERRO
            if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                controleEstado == BUS_OFF;
            } 
            else {
                count = 0;
                stuffing_enable = 0;
                if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                    controleEstado = SEND_ACTIVE_ERROR;
                }
                else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                    controleEstado = SEND_PASSIVE_ERROR;
                }            
            }            
        }
        else {
            if(ARB_wp == 1) {
                ARB_output = FRAME[count];
                count++;
            }

            if(new_sp == 1 && new_input != ARB_output && count != data_limit + 16) { //count != ack slot
                controleEstado = BIT_ERROR;
                break;
            }

            if(count == data_limit + 16) {
                stuffing_enable = 0;
            }

            if(count == data_limit + 29) { //count != ack slot
                eof = 1;
                controleEstado = INICIO;
            }          
        }        
        break;
    }
    case EXTENDED:
    {
        if(error_detected == 1) { //CONTROLE DE ERRO
            if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                controleEstado == BUS_OFF;
            } 
            else {
                count = 0;
                stuffing_enable = 0;
                if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                    controleEstado = SEND_ACTIVE_ERROR;
                }
                else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                    controleEstado = SEND_PASSIVE_ERROR;
                }            
            }            
        }
        else {
            if(ARB_wp == 1) {
                ARB_output = FRAME[count];
                count++;
            }

            if(new_sp == 1 && new_input != ARB_output && count != data_limit + 16) { //count != ack slot
                controleEstado = BIT_ERROR;
                break;
            }

            if(count == data_limit + 16) {
                stuffing_enable = 0;
            }

            if(count == data_limit + 29) { //count != ack slot
                eof = 1;
                controleEstado = INICIO;
            }
        }
        break;
    }
    case BIT_ERROR:
    {
        bit_error = 1;
        if(error_detected == 1) { //CONTROLE DE ERRO
            if(error_state == /*BUSS_OFF_CODE*/) { //Código de erro Bus_Off
                controleEstado == BUS_OFF;
            } 
            else {
                count = 0;
                stuffing_enable = 0;
                if (error_state == /*ACTIVE_ERROR_CODE*/) { //Código de erro ativo
                    controleEstado = SEND_ACTIVE_ERROR;
                }
                else if(error_state == /*PASSIVE_ERROR_CODE*/) { //Código de erro passivo
                    controleEstado = SEND_PASSIVE_ERROR;
                }            
            }            
        }
        break;
    }
    case SEND_ACTIVE_ERROR:
    {
        ARB_output = 0;

        if(count < 6 && ARB_wp == 1) {
            count++;
        }
        else if(count == 6 && ARB_wp == 1) {
            ARB_output = 1;
            controleEstado = INICIO;
        }
        break;
    }
    case SEND_PASSIVE_ERROR:
    {
        ARB_output = 1;

        if(count < 6 && ARB_wp == 1) {
            count++;
        }
        else if(count == 6 && ARB_wp == 1) {
            ARB_output = 1;
            controleEstado = INICIO;
        }
        break;
    }
    case BUS_OFF:
    {
        printf("WAIT SYSTEM RESET!!");
    }    
  }
}