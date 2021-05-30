/*
Projet Robot Filoguidé
module avertisseur sonore 
-> le robot emet un son d'une fréquence d'1kHz

Branchements sur le LPC2368 : 
        - haut parleur :    P0.26   (DAC)
*/

#include "LPC23xx.h"
#include <math.h>
#include <stdlib.h>

#define FCLK 12000000
#define FECH 36000      // frequence d'echantillonnage

#define VREF 3.3
#define PI 3.14
#define SIZE 100

int i_son = 0;
char sound = 1;         // variable qui sera modifié en fonction du besoin

unsigned int son[SIZE] = {512,534,467,578,422,623,379,666,336,708,295,748,255,787,217,824,182,858,149,890,119,918,92,944,68,966,47,985,30,1000,17,1012,7,1019,1,1023,0,1023,2,1019,8,1011,17,999,31,984,48,965,69,942,93,916,121,887,151,856,184,821,220,785,258,746,297,705,339,663,382,620,425,575,470,531,514,486,559,442,604,398,647,354,690,312,731,272,771,233,808,197,843,163,876,131,906,103,933,78,957,56,977,37};

void isrSon()__irq{
    if(sound){
        DACR = son[i_son]<<6;
        i_son++;
        if(i_son >= SIZE)
            i_son = 0;
    } else {
        i_son = 0;
    }
}

void initSon(){
    PWM1MR0 = FCLK/100;
    PWM1MR1 = 0.5 * PWM1MR0;
    PWM1PCR = 0x2;
    PINSEL3 = 1 << 5;       // 1.18
    PWM1MCR = 0x3;          // interrupt + reset    
    PWM1TCR = 0x9;
    PWM1LER = 0x3F;

    VICIntEnable = 1 << 8;
    VICVectAddr8 = (unsigned long)isrSon;
}

void initDAC(){
    PINSEL1 |= 1 << 21;       // 0.26 en DAC
}

void initLPC(){
    SCS = 1;
	initSon();
    initDAC();
}

int main(void){
    initLPC();
    while(1);
}