/*
ADC 
0.0 => P0.23
0.1 => P0.24
0.2 => P0.25
*/

#include "LPC23xx.h"

#define FCLK 12000000
#define FREQ_T1 50

volatile unsigned int ADC0=0;
volatile unsigned int ADC1=0;
volatile unsigned int ADC2=0;

int timer = 0;
int adc = 0;

void isrADC()__irq{
    adc++;
    AD0CR &= ~(1<<16);      // arret de la conversion
    ADC0 = (AD0DR0>>6)&0x3FF;
    ADC1 = (AD0DR1>>6)&0x3FF;
    ADC2 = (AD0DR2>>6)&0x3FF;
    VICVectAddr = 0;
}

void initADC(){
    PCONP |= 1 << 12;                           // alimentation ADC
    AD0CR |= 1 << 21;                           // PDN
    PINSEL1 |= (0x01<<14)|(0x01<<16)|(0x01<<18);         // ADC
    PINMODE1 |= (0x2<<14)|(0x2<<16)|(0x2<<18);   // no pull up or down 
    AD0CR |= 2 << 8;
    AD0INTEN = 0x4;
    AD0CR |= 0x7;
    VICIntEnable = 1 << 18;
    VICVectAddr18 = (unsigned long)isrADC;
}

void isrT1()__irq{
    timer++;
    AD0CR |= 1 << 16;
    T1IR = 0x3F;
    VICVectAddr = 0;
}

void initT1(){
    T1MR0 = 11999;//FCLK/FREQ_T1;
    T1MCR = 0x3;             // reset + interrupt 
    T1IR = 0x1;
    VICVectAddr5 = (unsigned long)isrT1;
    VICIntEnable = 1 << 5;
    T1TCR = 0x1;
}

void initLPC(){
    SCS = 1;
    initADC();
    initT1(); 
}

int main(){
    initLPC();
    while(1);
}
