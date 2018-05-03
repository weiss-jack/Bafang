/*
Bafand protocol analysis ; Down-Link
*/
#if defined(__AVR_ATmega2560__) // ATMEGA 2560 board
  HardwareSerial* BBS=&Serial1; // BBS sur Serial 1
#else
  #include <SoftwareSerial.h>  
  static SoftwareSerial mySerial1(10, 11); // Liaison 1 sur (10, 11)       
  SoftwareSerial* BBS =& mySerial1;
#endif

#include "Bafang-hack.h"

BAFANG_t BF_Down; 

/****************************************************************************************************
 * Bafang_Init() - Initialisation
 *
 ****************************************************************************************************/
#if defined(__AVR_ATmega2560__) // ATMEGA 2560 board
  void Bafang_Init (BAFANG_t* BF_ctx, HardwareSerial* PortSerie)
#else
  void Bafang_Init (BAFANG_t* BF_ctx, SoftwareSerial* PortSerie)
#endif
{
    BF_ctx->SerialPort = PortSerie;            // Liaison montante vers la console
    PortSerie->begin(1200);
}

void setup()
{
  //AssistLevel = 1;
  //WheelDiameter = 26;
  
  Serial.begin(115200);     //debug port
  Serial.println("Emulateur de console (liaison down)");
  Serial.print("Perimetre : ");Serial.println(WheelPerimeter);
  Bafang_Init(&BF_Down, BBS);
  delay (500); 
  Bafang_Command(&BF_Down, BF_CMD_LIGHT,0xF0);
  Bafang_Command(&BF_Down, BF_CMD_LEVEL,AssistLevel);
  Bafang_Command(&BF_Down, BF_CMD_WHEEL,WheelPerimeter); // normalent 200 (0xC8) (et non 207(0xCF) pour 26"
  Bafang_Command(&BF_Down, BF_CMD_LEVEL,AssistLevel);
  delay(200);
  //Serial.println("    0x01  ; 0x06 ; 0x07 ; 0X0B ; 0x0E ; 0x0F ;   0x1C  ;    0x1D    ;   0x31   ;     0x90    ");
  Serial.println("--> 0x01  ; 0x07 ; 0x0E ;    0x1D    ");
}
void loop()
{
  // #11 #01 --> 00 2A     Variable : 1er chiffre : 0, 1, 2 ou 3 ; 2eme chiffre identique au 2eme de #11 #1C
  // #11 #06 --> 00        Toujours = 0
  // #11 #07 --> 00        Variable : en rapport avec le courant ??
  // #11 #0B --> 01        Toujours égal à 1
  // #11 #0E --> 00        Variable : Position de la gachette (de 0x28 à 0xC8)
  // #11 #0F --> 00        Frein 1 si appuye
  // #11 #10 --> 00 00     Variable : Comptage de tours de roue depuis la mise spous tension (octet dédoublé)
  // #11 #1C --> 1F 64     Variable : 1er chiffre : 0x1F ; 2eme chiffre identique au 2eme de #11 #01 (de 0x24 à 0x29)
  // #11 #1D --> FF FF FE  Variable : en rapport avec la vitesse  ??
  // #11 #31 --> 30 30     Variable : Tension nominale (48V --> 0x30) ; varie un peu : 0x30, 0x31
  // #11 #90 --> 90 40 D0  Constant
  
 
  Serial_Input();
  
  Bafang_Request(&BF_Down, BF_GET_BAT) ;
  Battery_Percent =BF_Down.RxBuff[0]; 
  /*
  byte Test = 0xD0;
  //for(byte i=Test; i<Test+0x20; i++)
  for(byte i=0x21; i<0x32; i++)
  {
    Bafang_Request(&BF_Down, i) ;
    if (BF_Down.RxCnt >0)
    {
      Serial.print("Test : ");Serial.print(i,HEX);Serial.print(" --> "); 
      for(uint8_t j=0; j< BF_Down.RxCnt; j++)
      {
      Serial.print(BF_Down.RxBuff[j],HEX);Serial.print(" ");
      }
      Serial.println(" ");
    }
  } 
  */
  
  //Serial.println("    0x01  ; 0x06 ; 0x07 ; 0X0B ; 0x0E ;   0x1C  ;    0x1D    ");
  //Serial.println("--> 0x01  ; 0x07 ; 0x0E ;    0x1D    ");
  Serial.print("--> "); 
  Bafang_Request(&BF_Down, 0x01);
  Serial.print(BF_Down.RxBuff[0],HEX);Serial.print(" ");Serial.print(BF_Down.RxBuff[1],HEX);Serial.print("  ;  ");
  //Bafang_Request(&BF_Down, 0x06);
  //Serial.print(BF_Down.RxBuff[0],HEX);Serial.print("   ;  ");
  Bafang_Request(&BF_Down, 0x07);
  Serial.print(BF_Down.RxBuff[0],HEX);Serial.print("   ;  ");
  //Bafang_Request(&BF_Down, 0x0B);
  //Serial.print(BF_Down.RxBuff[0],HEX);Serial.print("   ;  ");
  Bafang_Request(&BF_Down, 0x0E) ;
  Serial.print(BF_Down.RxBuff[0],HEX);Serial.print("   ;  ");
  //Bafang_Request(&BF_Down, 0x1C);
  //Serial.print(BF_Down.RxBuff[0],HEX);Serial.print(" ");Serial.print(BF_Down.RxBuff[1],HEX);Serial.print("  ;  ");
  Bafang_Request(&BF_Down, 0x1D);
  Serial.print(BF_Down.RxBuff[0],HEX);Serial.print(" ");Serial.print(BF_Down.RxBuff[1],HEX);Serial.print(" ");Serial.print(BF_Down.RxBuff[2],HEX);Serial.print("  ;  ");
    
  Bafang_Request(&BF_Down, BF_GET_CURRENT) ;
  Current = BF_Down.RxBuff[0]/2; 

  Bafang_Request(&BF_Down, BF_GET_VOLTAGE) ;
  Voltage = BF_Down.RxBuff[0]; 
   
  Bafang_Request(&BF_Down, BF_GET_ERROR) ;
  //Serial.print("[0] : ");Serial.print(BF_Down.RxBuff[0]);Serial.print(" [1] : ");Serial.print(BF_Down.RxBuff[1]);
  //Serial.print(" [2] : ");Serial.print(BF_Down.RxBuff[2]);Serial.print(" [3] : ");Serial.println(BF_Down.RxBuff[3]);
  if (BF_Down.RxBuff[0] != 0x01)
  {
 //   Serial.print("Erreur : ");Serial.println(BF_Down.RxBuff[0]); 
  }
  Bafang_Request(&BF_Down, BF_GET_SPEED) ;
  WheelRPM = BF_Down.RxBuff[1]; 
  Speed = (WheelRPM*60*WheelPerimeter/100)/1000.;  
  delay(200);

  Serial.print("Niveau : ");Serial.print(AssistLevel);Serial.print(" ");
  Serial.print("Batterie : ");Serial.print(Battery_Percent);Serial.print(" % ");Serial.print("("); 
//  Serial.print(Voltage);Serial.print(" V)");Serial.print(" ; "); 
  Serial.print("Courant : ");Serial.print(Current);Serial.print(" A");Serial.print(" ; "); 
  Serial.print("Vitesse : ");Serial.print(Speed);Serial.print(" km/h");Serial.print("");

  Serial.println(" ");

}
/****************************************************************************************************
 * Serial_Input() - Lecture du port serie (chgt de niveau)
 *
 ***************************************************************************************************/
void Serial_Input(void)
{
   char Lecture;
   if (Serial.available()>0)
  {
    Lecture = Serial.read();
    //Serial.print("Lecture : " );
     
    while(Serial.available())  Serial.read(); // purge du buffer d'entree
    
    if (Lecture == 0x2B) // "+" en ASCII
    {
      if (AssistLevel <9) AssistLevel++; //   Serial.println("Plus");
    }
    else if (Lecture == 0x2D)
    {
      if (AssistLevel >0) AssistLevel--; //   Serial.println("Moins");
    }
    else if ((Lecture == 0x50)||((Lecture == 0x70))) AssistLevel = 10; // pieton
    else if ((Lecture >= 0+0x30)&&(Lecture < 9+0x30)) AssistLevel = Lecture-0x30;
    else Serial.println("Le niveau doit etre compris entre 0 et 9");
   
    Bafang_Command(&BF_Down, BF_CMD_LEVEL,AssistLevel); 
    delay(100);
  }  
}

/****************************************************************************************************
 * Bafang_Request() - Request data to BBS
 *
 ***************************************************************************************************/
void Bafang_Request(BAFANG_t* BF_ctx, byte Code)
{
    BF_Request(BF_ctx,Code);
    delay(100);
    long Attente = millis();
    BF_ctx->RxCnt=0;
    for(uint8_t i=0; i<BF_MAX_RXBUFF; i++) BF_ctx->RxBuff[i] = 0x00;  // effacement buffer
        
    while(BF_ctx->SerialPort->available())
    {
      BF_ctx->RxBuff[BF_ctx->RxCnt] = BF_ctx->SerialPort->read();
      BF_ctx->RxCnt++; 
      if ((millis()-Attente) > 200)
      {
        Serial.print("Test du code 0x11 ");Serial.print(Code,HEX);Serial.print(" non-concluant");
        break; //rien n'a  ete recu
      }
   }        
   delay(100);
}
/****************************************************************************************************
 * Bafang_Command() - Send a command to BBS
 *
 ***************************************************************************************************/
void Bafang_Command(BAFANG_t* BF_ctx, byte Code,int Value)
{
  byte count;
  TxBuff[0] = BF_CMD_COMMAND; // 0x16
  switch(Code)
     {
       case BF_CMD_LIGHT:
        TxBuff[1] = BF_CMD_LIGHT;
        TxBuff[2] = Value;
        count = 2;
       break;
     
       case BF_CMD_LEVEL:
        TxBuff[1] = BF_CMD_LEVEL;
        TxBuff[2] = BF_LEVEL[Value];
        TxBuff[3] = TxBuff[0]+TxBuff[1]+TxBuff[2];
        count = 3;
       break;
       
       case BF_CMD_WHEEL:
        TxBuff[1] = BF_CMD_WHEEL;
        TxBuff[2] = (Value>>8);
        TxBuff[3] = (Value&0xff);
        TxBuff[4] = TxBuff[0]+TxBuff[1]+TxBuff[2]+TxBuff[3];
        count = 4;
       break;
       
       default:
        count = 0;
       break;    
     }
     for(int i=0; i<= count; i++) BF_ctx->SerialPort->write(TxBuff[i]); // envoi
}


void BF_Request(BAFANG_t* BF_ctx,byte Code)
{
    BF_ctx->SerialPort->write(BF_CMD_REQUEST); // 0x11
    BF_ctx->SerialPort->write(Code);
}



