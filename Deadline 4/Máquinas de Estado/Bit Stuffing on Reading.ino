#define INICIO '0'
#define STUFF_0 '1'
#define STUFF_1 '2'
#define ERROR '3'

int count;
int input, samplePoint, stuffing_enable; //INPUTS
int newInput, new_sp, stuffing_error;// OUTPUTS
char controleEstado;

void setup() {
    controleEstado = INICIO;  
}

void loop() {
  switch(controleEstado){
    case INICIO:
    {
        stuffing_error = 0;
        count = 0;
        newInput = input;
        new_sp = samplePoint;

        if(stuffing_enable == 1 && samplePoint == 1 && input == 0) {
            count++;
            newInput = input;
            new_sp = samplePoint;
            controleEstado = STUFF_0;
        } 
        else if(stuffing_enable == 1 && samplePoint == 1 && input == 1) {
            count++;
            newInput = input;
            new_sp = samplePoint;
            controleEstado = STUFF_1;
        }
        break;
    }

    case STUFF_0:
    {
        new_sp = 0;
        
        if(stuffing_enable == 0) {  //Essa parte eu repeti as intruções do estado INÍCIO só pra prevenir de alguma possível dessincronização.
            count = 0;              //Quando a gente for testar a gente vê se precisa repetir ou se basta só setar a variável controleEstado = INICIO
            newInput = input;
            new_sp = samplePoint;
            controleEstado = INICIO;
        }
        else if(stuffing_enable == 1 && samplePoint == 1 && input == 0 && count < 5) {
            count++;
            newInput = input;
            new_sp = samplePoint;          
        }
        else if(stuffing_enable == 1 && samplePoint == 1 && input == 1 && count < 5) {
            count = 1;
            newInput = input;
            new_sp = samplePoint;
            controleEstado = STUFF_1;
        }
        else if(stuffing_enable == 1 && samplePoint == 1 && input == 0 && count == 5) {
            controleEstado = ERROR;
        }
        break;
    }

    case STUFF_1: 
    {
        new_sp = 0;
        
        if(stuffing_enable == 0) {  //Essa parte eu repeti as intruções do estado INÍCIO só pra prevenir de alguma possível dessincronização.
            count = 0;              //Quando a gente for testar a gente vê se precisa repetir ou se basta só setar a variável controleEstado = INICIO
            newInput = input;
            new_sp = samplePoint;
            controleEstado = INICIO;
        }
        else if(stuffing_enable == 1 && samplePoint == 1 && input == 1 && count < 5) {
            count++;
            newInput = input;
            new_sp = samplePoint;          
        }
        else if(stuffing_enable == 1 && samplePoint == 1 && input == 0 && count <= 5) {
            count = 1;
            newInput = input;
            new_sp = samplePoint;
            controleEstado = STUFF_0;
        }
        else if(stuffing_enable == 1 && samplePoint == 1 && input == 1 && count == 5) {
            controleEstado = ERROR;
        }
        break;
    }

    case ERROR:
    {
        stuffing_error = 1;
        controleEstado = INICIO;
        break;
    }
  }
}