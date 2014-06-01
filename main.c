#include <c8051f310.h>
#include <stdio.h>
#include <math.h>
#define SYSCLK      24500000/8
#define BAUDRATE        9600
sbit DATA = P3^2;
sbit LED = P3^3;
unsigned char xdata ROM[8];
unsigned char xdata tDegrees[8];
char tmp;

void SYSCLK_Init(void);
void UART0_Init(void);
void PORT_Init(void);
void Timer2_Init(int);
void Delay(int D);
void Reset();
void ReadData(unsigned char n, unsigned char* ucData);
void SendBit(unsigned char cbit);
void SendByte(unsigned char byte);
void ReadRom(unsigned char* D);
unsigned char ReadNBits(unsigned char n);
unsigned char ReadByte();
float decodeData(unsigned char* byte);

void main(void) {
    unsigned char inputcharacter;
    PCA0MD &= ~0x40;    
    PORT_Init();        
    SYSCLK_Init();      
    UART0_Init();       
    while (1) {
        printf("\nPress any key to start conversion: ");
        inputcharacter = getkey();
        ReadRom(ROM);
printf ("\nds18b20 is on line\nS/N: %bx %bx %bx %bx %bx %bx",
ROM[6],ROM[5],ROM[4],ROM[3],ROM[2],ROM[1]);
        Reset();
        SendByte(0xcc);
        SendByte(0x44);
        Delay(750);
        Reset();
        SendByte(0xcc);
        SendByte(0xbe);
        ReadData(8, tDegrees);
        printf("\n%f C", decodeData(tDegrees));

    }
}



void PORT_Init(void) {
    P0MDOUT |= 0x10;    
    XBR0 = 0x01;                           
    XBR1 = 0x40;        
    P3MDOUT = 0x08;     
}

void SYSCLK_Init(void) {
    OSCICN |= 0x00;     
    RSTSRC = 0x04;
}

void UART0_Init(void) {
    SCON0 = 0x10; 
    if (SYSCLK / BAUDRATE / 2 / 256 < 1) {
        TH1 = -(SYSCLK / BAUDRATE / 2);
        CKCON &= ~0x0B; // T1M = 1; SCA1:0 = xx
        CKCON |= 0x08;
    } else if (SYSCLK / BAUDRATE / 2 / 256 < 4) {
        TH1 = -(SYSCLK / BAUDRATE / 2 / 4);
        CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 01                  
        CKCON |= 0x01;
    } else if (SYSCLK / BAUDRATE / 2 / 256 < 12) {
        TH1 = -(SYSCLK / BAUDRATE / 2 / 12);
        CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 00
    } else {
        TH1 = -(SYSCLK / BAUDRATE / 2 / 48);
        CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 10
        CKCON |= 0x02;
    }
    TL1 = TH1;
    TMOD &= ~0xf0;
    TMOD |= 0x20;
    TR1 = 1;
    TI0 = 1;
}

void Delay(int D) 
{
    long i;
    long F = (long) ((D * 10) >> 8);
    for (i = 0; i < F; i++);
}

void Reset() {
    DATA = 0;
    Delay(500);
    DATA = 1;
    Delay(50);
    Delay(200);
    DATA = 1;
}

void ReadRom(unsigned char* D) {
    Reset();
    SendByte(0x33);
    ReadData(8, D);
}

void SendByte(unsigned char byte) {
    int i;
    for (i = 0; i < 8; i++) {
        SendBit(byte & 0x01);
        byte = byte >> 1;
    }
}

void SendBit(unsigned char cbit) {
    DATA = 0;
    tmp = tmp;
    tmp = tmp;
    DATA = cbit;
    Delay(90);
    DATA = 1;
    tmp = tmp;
    tmp = tmp;
}

void ReadData(unsigned char n, unsigned char* ucData) {
    unsigned char i = 0;
    while (i < n) {
        ucData[i] = ReadByte();
        i++;
    }
}

unsigned char ReadByte() {
    return ReadNBits(8);
}

unsigned char ReadNBits(unsigned char n) {
    unsigned char tmpValue = 0;
    unsigned char retValue = 0;
    unsigned char i = 0;
    while (i < n) {
        DATA = 0;
        tmp = tmp;
        tmp = tmp;
        DATA = 1;
        tmp = tmp;
        tmpValue = 0x01 & DATA;
        retValue += tmpValue << i;
        Delay(80);
        i++;
    }
    return retValue;
}

float decodeData(unsigned char* byte) {
    float temp = 0;
    int i;
    for (i = 0; i < 8; i++) {
        temp = temp + ((byte[0] >> i)&0x01) * pow(2, i - 4);
    }
    for (i = 0; i < 3; i++) {
        temp = temp + ((byte[1] >> i)&0x01) * pow(2, i + 4);
    }
    if ((byte[1] >> 3)&0x01) {
        temp = -temp;
    }
    return temp;
}
