#define WAIT_CRC_REQUEST '0'
#define NEXT_1 '1'
#define XOR '2'
#define CRC_RESULT '3'

int count, start_data, data_limit, PARTIAL_CRC[118], POLYNOMIAL[15] = {1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1};
int PARTIAL[103], CRC_CALC[15]; //INPUTS
int CRC_CALC[15];// OUTPUTS
char controleEstado;

void setup() {
    controleEstado = WAIT_CRC_REQUEST;
}

void loop() {
  switch(controleEstado){
    case INICIO:
    {
        if(new_frame == 1) {
            frame_ready = 0;
            if(IDE == 0) {
                controleEstado = BASE_FORMAT;
            }
            else {
                controleEstado = EXTENDED_FORMAT;
            }  
        }
        break;
    }
    case BASE_FORMAT:
    {
        FRAME[0] = 0; //(SOF)

        for (int i = 1; i <= 11; i++) { //ID
            FRAME[i] = ID[i-1];
        }

        FRAME[12] = RTR;
        FRAME[13] = IDE;
        FRAME[14] = 0; //r0

        for (int i = 15; i <= 18; i++) { //DLC
            FRAME[i] = DLC[i-15];
        }

        if(RTR == 0) {
            start_data = 19;
            data_limit = 18 + int(DLC); //ver a função pra transformar o DLC pra inteiro em C++
            controleEstado = DATA_FIELD;
        }
        else if(RTR == 1) {
            data_limit = 19;
            pt_counter = data_limit + 1;

            for (int i = 0; i <= 102; i++) {
                PARTIAL[i] = FRAME[i];
            }

            crc_req = 1;
            controleEstado = CRC_WAIT;
        }
        break;
    }
    case EXTENDED_FORMAT:
    {
        FRAME[0] = 0; //(SOF)
        for (int i = 1; i <= 11; i++) { //ID parte 1
            FRAME[i] = ID[i-1];
        }

        FRAME[12] = 1; //SRR
        FRAME[13] = IDE;
        for (int i = 14; i <= 31; i++) { //ID parte 2
            FRAME[i] = ID[i-3];
        }

        FRAME[32] = RTR; 
        FRAME[33] = 0; //r1
        FRAME[34] = 0; //r0
        for (int i = 35; i <= 38; i++) {
            FRAME[i] = DLC[i-35];
        }

        if(RTR == 0) {
            start_data = 39;
            data_limit = 38 + int(DLC); //ver a função pra transformar o DLC pra inteiro em C++
            controleEstado = DATA_FIELD;
        }
        else if(RTR == 1) {
            data_limit = 39;
            pt_counter = data_limit + 1;

            for (int i = 0; i <= 102; i++) {
                PARTIAL[i] = FRAME[i];
            }

            crc_req = 1;
            controleEstado = CRC_WAIT;
        }
        break;
    }
    case DATA_FIELD: 
    {
        payload_idx = 0;
        for (int i = start_data; i <= data_limit; i++) { //PAYLOAD
            FRAME[i] = PAYLOAD[payload_idx];
            payload_idx++;
        }

        pt_counter = data_limit + 1;
        for (int i = 0; i <= 102; i++) {
                PARTIAL[i] = FRAME[i];
            }
        
        crc_req = 1;
        break;
    }
    case CRC_WAIT:
    {
        if(crc_ready == 1) {
            controleEstado = CRC_ACK_EOF;
        }
        break;
    }
    case CRC_ACK_EOF:
    {
        crc_req = 0;
        for (int i = data_limit; i <= data_limit + 14; i++){
            FRAME[i] = CRC_CALC[i-data_limit];
        }

        FRAME[data_limit+15] = 1; //CRC delimiter
        FRAME[data_limit+16] = 1; //ACK slot
        FRAME[data_limit+17] = 1; //ACK delimiter
        for (int i = data_limit + 18; i <= data_limit + 24; i++) {
            FRAME[i] = 1; //EOF
        }

        frame_ready = 1;
        controleEstado = INICIO;
        break;
    }
    
  }
}