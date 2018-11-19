#include <stdio.h>
#define INICIO '0'
#define SOF '1'
#define IDENTIFIER_A '2'
#define RTR_SRR '3'
#define IDE '4'
#define IDENTIFIER_B '5'
#define RTR_EXT '6'
#define r1 '7'
#define r0 '8'
#define STAND_BY '9'
#define DLC '10'
#define DATA_FIELD '11'
#define CRC '12'
#define CRC_delimiter '13'
#define ACK_slot '14'
#define ACK_delimiter '15'
#define EOF '16'
#define ERROR '17'

int count, arb, crc[15], crc_ok, ack_d, crc_error, format_error;
int sampled_bit, lost_arbitration, calc_crc[15], crc_ready;// INPUTS
int ack, ide, ID[30], rtr, srr, PAYLOAD_SIZE[4], PAYLOAD[64], PARTIAL[103], i, stuffing_enable, eof, crc_req, stuffing_enable; //OUTPUTS
char controleEstado;

void setup() {
    controleEstado = INICIO;  
    for (int j = 0; j < 103; j++) {
        if(j < 4) {
            PAYLOAD_SIZE[j] = 0;
        }
        
        if(j < 30) {
            ID[j] = 0;
        }

        if(j < 64) {
            PAYLOAD[j] = 0;
        }

        PARTIAL[j] = 0;
    }

    rtr = 0;
    i = 0; //PT_COUNTER
    ide = 0;
    ack = 1;
    stuffing_enable = 0;
}

void loop() {
  switch(controleEstado){
    case INICIO:
    {
        eof = 1;
        arb = 0;
        if(sampled_bit == 0) {
            i = 0;
            controleEstado = SOF;
        }
        break;
    }
    case SOF:
    {
        eof = 0;
        stuffing_enable = 1;
        PARTIAL[i] = sampled_bit;
        i++;
        count = 0;

        if(lost_arbitration == 1 && arb == 0) {
            arb = 1;
        }
        
        controleEstado = IDENTIFIER_A;
        break;
    }
    case IDENTIFIER_A:
    {
        ID[count] = sampled_bit;
        count++;
        PARTIAL[i] = sampled_bit;
        i++;

        if(lost_arbitration == 1 && arb == 0) {
            arb = 1;
        }
        
        if(count == 11) {
            controleEstado = RTR_SRR;
        }
        break;
    }
    case RTR_SRR: 
    {
        rtr = sampled_bit;
        PARTIAL[i] = sampled_bit;
        i++;

        if(lost_arbitration == 1 && arb == 0) {
            arb = 1;
        }

        controleEstado = IDE;
        break;
    }
    case IDE:
    {
        ide = sampled_bit;
        PARTIAL[i] = sampled_bit;
        i++;

        if(lost_arbitration == 1 && arb == 0) {
            arb = 1;
        }

        if(ide == 1) {
            controleEstado = IDENTIFIER_B;
        }
        else if(ide == 0) {
            controleEstado = r0;
        }
        break;
    }
    case IDENTIFIER_B:
    {
        ID[count] = sampled_bit;
        count++;
        PARTIAL[i] = sampled_bit;
        i++;

        if(lost_arbitration == 1 && arb == 0) {
            arb = 1;
        }

        if(count == 30) {
            controleEstado = RTR_EXT;
        }
        break;
    }
    case RTR_EXT:
    {
        srr = rtr;
        rtr = sampled_bit;
        PARTIAL[i] = sampled_bit;
        i++;

        if(lost_arbitration == 1 && arb == 0) {
            arb = 1;
        }

        controleEstado = r1;
        break;
    }
    case r1:
    {
        PARTIAL[i] = sampled_bit;
        i++;
        controleEstado = r0;
        break;
    }
    case r0:
    {
        PARTIAL[i] = sampled_bit;
        i++;

        if(arb == 0) {
            controleEstado = STAND_BY;
        }
        else if(arb == 1) {
            count = 0;
            controleEstado = DLC;
        }
        break;
    }
    case DLC:
    {
        PAYLOAD_SIZE[count] = sampled_bit;
        count++;
        PARTIAL[i] = sampled_bit;
        i++;

        if(count > 3 && rtr == 0) {
            count = 0;
            PAYLOAD_SIZE = min(PAYLOAD_SIZE, 8); //ver função em c++ para fazer a conversão de binário p/ decimal e vice-versa
            controleEstado = DATA_FIELD;
        }
        else if(count > 3 && rtr == 1) {
            count = 0;
            controleEstado = CRC;
        }
        break;
    }
    case DATA_FIELD:
    {
        PAYLOAD[count] = sampled_bit;
        count++;
        PARTIAL[i] = sampled_bit;
        i++;

        if(count > (int(PAYLOAD_SIZE) * 8) - 1) { //ver função em c++ que converte binário para decimal
            count = 0;
            crc_req = 1;
            controleEstado = CRC;
        }
        break;
    }
    case CRC:
    {
        crc[count] = sampled_bit;
        count++;

        if(count == 15) {
            crc_req = 0;
            stuffing_enable = 0;
            controleEstado = CRC_delimiter;
        }
        break;
    }
    case CRC_delimiter:
    {
        if(crc_ready == 1){
            if(crc == calc_crc) { //comparar 2 arrays em c++
                    crc_ok = 1;
            }
            else {
                crc_ok = 0;
            }
            controleEstado = ACK_slot;
        }
        break;
    }
    case ACK_slot:
    {
        ack = !crc_ok;

        if(ack == 1) {
            crc_error = 1;
            controleEstado = ERROR;
        }
        else if(ack == 0) {
            controleEstado = ACK_delimiter;
        }
        break;
    }
    case ACK_delimiter:
    {
        ack_d = sampled_bit;

        if(ack_d == 0) {
            format_error = 1;
            controleEstado = ERROR;
        }
        else if(ack_d == 1) {
            count = 0;
            controleEstado = EOF;
        }
        break;
    }
    case EOF:
    {
        count++;
        if(count < 7 && sampled_bit == 0)
        {
            format_error = 1;
            controleEstado = ERROR;
        }
        else if(count == 7) {
            controleEstado = INICIO;
        }
        break;
    }
  }
}