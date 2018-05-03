#ifndef BAFANG-HACK_H
#define BAFANG-HACK_H

#define BF_MAX_RXBUFF 8
#define BF_MAX_TXBUFF 5

// Commandes
#define BF_CMD_REQUEST 0x11
#define BF_CMD_COMMAND 0x16
#define BF_CMD_LEVEL 0x0B
#define BF_CMD_LIGHT 0x1A
#define BF_CMD_WHEEL 0x1F
// Requetes
#define BF_GET_SPEED 0x20
#define BF_GET_ERROR 0x08
#define BF_GET_BAT 0x11
#define BF_GET_CURRENT 0x0A
#define BF_GET_VOLTAGE 0x31

#define BF_LIGHTON 0xF0
#define BF_LIGHTOFF 0xF1

#define BF_DISPLAYTIMEOUT 10

uint8_t TxBuff[BF_MAX_TXBUFF];

byte BF_LEVEL[11] = {0x00, 0x01, 0x0B, 0x0C, 0x0D, 0x02, 0x15, 0x16, 0x17, 0x03, 0x06};

uint8_t  AssistLevel = 1;               // 0..9 (+P) Power Assist Level
byte Battery_Percent;
byte Voltage;
byte WheelDiameter = 26; // en pouces
byte WheelPerimeter = WheelDiameter*3.14159*2.54; // en centimetres
//byte WheelPerimeter = 200; // en centimetres (200 pour une roue de 26"")
long WheelRPM;
float Current;
float Speed;

typedef struct
{
//    Parameters received from display in operation mode:
    uint8_t  AssistLevel;               // 0..9 Power Assist Level 
    uint8_t  Headlight;                 // BF_HEADLIGHT_OFF / BF_HEADLIGHT_ON 
    uint8_t  PushAssist;                // BF_PUSHASSIST_OFF / BF_PUSHASSIST_ON
    uint16_t Wheeldiameter;             // Wheel Diameter
}RX_PARAM_t;

typedef struct
{
#if defined(__AVR_ATmega2560__) // ATMEGA 2560 board
    HardwareSerial* SerialPort;
#else
    SoftwareSerial* SerialPort;
#endif
    
    uint8_t         RxState;
    uint32_t        LastRx;
    uint8_t         RxBuff[BF_MAX_RXBUFF];
    uint8_t         RxCnt;
    uint8_t         InfoLength;

    RX_PARAM_t      Rx;

}BAFANG_t;


#endif // BAFANG-HACK

