void setup() {
  #define SYNC_SEG '0'
  #define TSEG1 '1'
  #define TSEG2 '2'
  #define sjw 1
  #define t1 8
  #define t2 -7
  
  int count, countLimit1, countLimit2, writingPoint, hardSync, reSync, sampledBit, phaseError, compensates;
  char controleEstado;
  controleEstado = SYNC_SEG;  
}

void loop() {
  switch(controleEstado){
    case SYNC_SEG:
    {
      count = 0;
      writingPoint = 1;
      controleEstado = TSEG1;
      countLimit1 = t1;
      countLimit2 = 0;
      compensates = 0;
      break;
    }

    case TSEG1:
    {
      writingPoint = 0;
      phaseError = count;    
      compensates = 0; 

      if(hardSync == 1)
      {
        count = 0;
        writingPoint = 1;
        controleEstado = TSEG1;
      }
      else if(reSync == 1)
      {
        if(sjw < phaseError)
          countLimit1 = countLimit1 + sjw;
        else 
          countLimit1 = countLimit1 + phaseError;
      }
      
      if(count < countLimit1)
        count++;
      else if(count == countLimit1) 
      {
        sampledBit = RX;
        samplePoint = 1;
        count = t2;
        controleEstado = TSEG2;       
      }
      break;
    }

    case TSEG2:
    {
      samplePoint = 0;
      phaseError = count * -1;

      if(hardSync == 1)
      {
        count = 0;
        writingPoint = 1;
        controleEstado = TSEG1;
      }
      else if(reSync == 1)
      {
        if(sjw < phaseError)
          countLimit2 = countLimit2 - sjw;
        else 
        {
          countLimit2 = countLimit2 - phaseError;
          compensates = 1;
        }
      }
     
     
     if(count < countLimit2)
      count++;
     else if(count == countLimit2)
     {
      if(compensates == 0)
        controleEstado = SYNC_SEG;
      else
      {
        count = 0;
        writing_point = 1;
        controleEstado = TSEG1;
      }
     }
     break;
    }
  }
}