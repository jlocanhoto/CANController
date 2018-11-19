#define INICIO '0'
#define STUFF_0 '1'
#define STUFF_1 '2'

int count;
int ARB_output, writingPoint, stuffing_enable; //INPUTS
int output, ARB_wp;// OUTPUTS
char controleEstado;

void setup() {
    controleEstado = INICIO;  
}

void loop() {
  switch(controleEstado){
    case INICIO:
    {
        count = 0;
        output = ARB_output;
        ARB_wp = writingPoint;

        if(stuffing_enable == 1 && writingPoint == 1 && ARB_output == 0) {
            count++;
            output = ARB_output;
            ARB_wp = writingPoint;
            controleEstado = STUFF_0;
        } 
        else if(stuffing_enable == 1 && writingPoint == 1 && ARB_output == 1) {
            count++;
            output = ARB_output;
            ARB_wp = writingPoint;
            controleEstado = STUFF_1;
        }
        break;
    }

    case STUFF_0:
    {
        ARB_wp = 0;
        
        if(stuffing_enable == 0) {  //Essa parte eu repeti as intruções do estado INÍCIO só pra prevenir de alguma possível dessincronização.
            count = 0;              //Quando a gente for testar a gente vê se precisa repetir ou se basta só setar a variável controleEstado = INICIO
            output = ARB_output;
            ARB_wp = writingPoint;
            controleEstado = INICIO;
        }
        else if(stuffing_enable == 1 && writingPoint == 1 && ARB_output == 0 && count < 5) {
            count++;
            output = ARB_output;
            ARB_wp = writingPoint;          
        }
        else if(stuffing_enable == 1 && writingPoint == 1 && ARB_output == 1 && count <= 5) {
            count = 1;
            output = ARB_output;
            ARB_wp = writingPoint;
            controleEstado = STUFF_1;
        }
        else if(stuffing_enable == 1 && writingPoint == 1 && ARB_output == 0 && count == 5) {
            count = 0;
            output = 1;
            ARB_wp = 0;
        }
        break;
    }

    case STUFF_1: 
    {
        ARB_wp = 0;
        
        if(stuffing_enable == 0) {  //Essa parte eu repeti as intruções do estado INÍCIO só pra prevenir de alguma possível dessincronização.
            count = 0;              //Quando a gente for testar a gente vê se precisa repetir ou se basta só setar a variável controleEstado = INICIO
            output = ARB_output;
            ARB_wp = writingPoint;
            controleEstado = INICIO;
        }
        else if(stuffing_enable == 1 && writingPoint == 1 && ARB_output == 1 && count < 5) {
            count++;
            output = ARB_output;
            ARB_wp = writingPoint;          
        }
        else if(stuffing_enable == 1 && writingPoint == 1 && ARB_output == 0 && count <= 5) {
            count = 1;
            output = ARB_output;
            ARB_wp = writingPoint;
            controleEstado = STUFF_1;
        }
        else if(stuffing_enable == 1 && writingPoint == 1 && ARB_output == 1 && count == 5) {
            count = 0;
            output = 0;
            ARB_wp = 0;
        }
        break;
    }
  }
}