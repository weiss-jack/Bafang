/*
Bafand protocol analysis, Up-link
*/

#if defined(__AVR_ATmega2560__) // ATMEGA 2560 board
  HardwareSerial* CONSOLE=&Serial1; // afficheur sur Serial 1
#else
  #include <SoftwareSerial.h>
  static SoftwareSerial mySerial1(10, 11); // Liaison 1 sur (10, 11)        
  SoftwareSerial* CONSOLE =& mySerial1;
#endif

#include "Bafang-hack.h"

// Definitions
#define RXSTATE_WAITGAP     0 //waiting for gap between messages to reset rx buffer
#define RXSTATE_STARTCODE   1 //waiting for startcode
#define RXSTATE_REQUEST     2 //request startcode received, waiting for request code
#define RXSTATE_INFO        3 //info startcode received, waiting for info code
#define RXSTATE_INFOMESSAGE 4 //info code received, waiting for info message
#define RXSTATE_DONE        5 //command received


BAFANG_t BF_Up;

/****************************************************************************************************
 * Bafang_Init() - Initialisation port et variables
 *
 ****************************************************************************************************/
#if defined(__AVR_ATmega2560__) // ATMEGA 2560 board
  void Bafang_Init (BAFANG_t* BF_ctx, HardwareSerial* PortSerie)
#else
  void Bafang_Init (BAFANG_t* BF_ctx, SoftwareSerial* PortSerie)
#endif
{
    BF_ctx->SerialPort = PortSerie;            // Liaison montante vers la console
    BF_ctx->RxState     = RXSTATE_WAITGAP;
    BF_ctx->LastRx      = millis();

    for(uint8_t i=0; i<BF_MAX_RXBUFF; i++)BF_ctx->RxBuff[i] = 0x00; // effacement du buffer

    PortSerie->begin(1200);
}

void setup()
{
  Serial.begin(115200);     //debug port
   
  Serial.println("Emulateur de console (liaison down)");
  Serial.print("Perimetre : ");Serial.println(WheelPerimeter);
  Bafang_Init(&BF_Up, CONSOLE);

  delay (500); 
}
void loop()
{
  Battery_Percent = 50;
  Current = 30;
  //WheelRPM = 0xC9;  // 25 km/h
  Speed = 25.;        //vitesse en km/h
  WheelRPM = 1000*Speed/(60*WheelPerimeter/100);
  
  Bafang_Up(&BF_Up);
    
}

/****************************************************************************************************
 * Bafang_Up() - Communication avec la console
 *
 ***************************************************************************************************/
void Bafang_Up(BAFANG_t* BF_ctx)
{
   // uint8_t  i;  
    //wait for gap
    if(BF_ctx->RxState == RXSTATE_WAITGAP) //waiting for start code
    {
      while(BF_ctx->SerialPort->available())
      {
        BF_ctx->SerialPort->read();
        BF_ctx->LastRx = millis();
      }
      
      if (millis()-BF_ctx->LastRx>BF_DISPLAYTIMEOUT) //gap detected
      {
        BF_ctx->RxState++; ///go to next state
        BF_ctx->RxCnt=0;
      }
    }
    
    // Search for Start Code
    if(BF_ctx->RxState == RXSTATE_STARTCODE) //waiting for start code
    {
      if(BF_ctx->SerialPort->available())
      {
          BF_ctx->LastRx = millis();
          BF_ctx->RxBuff[0]=BF_ctx->SerialPort->read();
          if(BF_ctx->RxBuff[0]==BF_CMD_REQUEST) //valid request startcode detected
          {
              BF_ctx->RxCnt = 1;
              BF_ctx->RxState = RXSTATE_REQUEST;
          }
          else if(BF_ctx->RxBuff[0]==BF_CMD_COMMAND) //valid info startcode detected
          {
              BF_ctx->RxCnt = 1;
              BF_ctx->RxState = RXSTATE_INFO;
          }
          else BF_ctx->RxState == RXSTATE_WAITGAP;
      }
    }
    
    if(BF_ctx->RxState == RXSTATE_REQUEST) //we are waiting for request code
    {
      if(BF_ctx->SerialPort->available()) //request code received
      {
        BF_ctx->RxBuff[BF_ctx->RxCnt] = BF_ctx->SerialPort->read();
        BF_ctx->RxCnt++;            
        BF_ctx->LastRx = millis();
        switch (BF_ctx->RxBuff[1]) // analyze and send correct answer
        {
         case BF_GET_SPEED:            
          TxBuff[0]=(WheelRPM>>8);
          TxBuff[1]=(WheelRPM&0xff);
          TxBuff[2]=TxBuff[0]+TxBuff[1]+32;
          BF_sendmessage(BF_ctx,3);
          Serial.print(" Vitesse :" );Serial.println(Speed);
         break;
              
         case BF_GET_ERROR:
          TxBuff[0]=1;
          BF_sendmessage(BF_ctx,1);
         break;
              
         case BF_GET_BAT:
          TxBuff[0]=Battery_Percent;
          TxBuff[1]=Battery_Percent;
          BF_sendmessage(BF_ctx,2);
         break;
              
         case BF_GET_CURRENT:
          TxBuff[0]=Current;
          TxBuff[1]=Current;
          BF_sendmessage(BF_ctx,2);
         break;
              
         case BF_GET_VOLTAGE:
          TxBuff[0]=Voltage;
          TxBuff[1]=Voltage;
          BF_sendmessage(BF_ctx,2);
         break;
        }
        BF_ctx->RxState = RXSTATE_WAITGAP;  //reset state machine
      }       
    }
    
    if(BF_ctx->RxState == RXSTATE_INFO) //we are waiting for info code
    {
      if(BF_ctx->SerialPort->available()) //info code received
      {
        BF_ctx->RxBuff[BF_ctx->RxCnt] = BF_ctx->SerialPort->read();
        BF_ctx->RxCnt++;            
        BF_ctx->LastRx = millis();
        switch (BF_ctx->RxBuff[1])   //analyze info code and set correct bytes to receive
        {
          case BF_CMD_LEVEL:
           BF_ctx->InfoLength=4;  //level message has length of 4 bytes
           BF_ctx->RxState++;
          break;
          
          case BF_CMD_LIGHT:
           BF_ctx->InfoLength=3;  //light message has length of 3 bytes
           BF_ctx->RxState++;
          break;
          
          case BF_CMD_WHEEL: //wheeldiameter message has length of 5 bytes
           BF_ctx->InfoLength=5;
           BF_ctx->RxState++;
          break;
          
          default:
           BF_ctx->RxState=RXSTATE_WAITGAP; //not a valid message -> reset state machine
          break;
        }
      }        
    }
    
    if (BF_ctx->RxState == RXSTATE_INFOMESSAGE) //we are waiting for info message
    {
      while(BF_ctx->SerialPort->available()&&BF_ctx->RxCnt<BF_ctx->InfoLength) //read info message
      {
        BF_ctx->RxBuff[BF_ctx->RxCnt] = BF_ctx->SerialPort->read();
        BF_ctx->RxCnt++;            
        BF_ctx->LastRx = millis();
      }
      
      if (BF_ctx->RxCnt==BF_ctx->InfoLength) //info message complete --> analyze
      {
        switch (BF_ctx->RxBuff[1])
        {
          case BF_CMD_LEVEL:
          if (BF_ctx->RxBuff[3]==BF_ctx->RxBuff[0]+BF_ctx->RxBuff[1]+BF_ctx->RxBuff[2]) //checksum is correct, set poti_stat
          {
            BF_ctx->Rx.PushAssist=0;
            for(int i=0; i<= 10; i++)
            {
              if (BF_ctx->RxBuff[2] == BF_LEVEL[i]) BF_ctx->Rx.AssistLevel=i;
            }
          }
          Serial.print( " Niveau : "); Serial.println(BF_ctx->Rx.AssistLevel);
          break;
         
          case BF_CMD_LIGHT:
          BF_ctx->Rx.Headlight=(BF_ctx->RxBuff[2]==BF_LIGHTON);
          break;
          
          case BF_CMD_WHEEL:
          BF_ctx->Rx.Wheeldiameter=BF_ctx->RxBuff[2]*256+BF_ctx->RxBuff[3];
          Serial.print( " Perimetre roue : "); Serial.println(BF_ctx->RxBuff[3]); 
          break;
        }     
       BF_ctx->RxState = RXSTATE_STARTCODE; 
      }
    }
}
void BF_sendmessage(BAFANG_t* BF_ctx,uint8_t count)
{
  for(int i=0; i<count; i++) BF_ctx->SerialPort->write(TxBuff[i]);
}




