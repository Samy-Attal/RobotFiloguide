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

#define SIZE 97

int i_son = 0;
char sound = 1;         // variable qui sera modifié en fonction du besoin

unsigned int son[SIZE] = {567,622,676,728,778,824,867,905,938,967,989,1007,1017,1022,1022,1014,1001,982,957,926,891,852,808,760,710,657,603,547,491,435,380,327,276,227,183,141,105,73,47,26,11,2,0,2,11,26,48,74,106,143,184,230,278,329,383,438,494};

void isrSon()__irq{
    if(sound){
        DACR = son[i_son]<<6;
        i_son++;
        if(i_son >= SIZE)
            i_son = 0;
    } else {
        i_son = 0;
    }
    VICVectAddr = 0;
    PWM1IR = 0xFF;
}

void initSon(){
    PWM1MR0 = FCLK/100000;
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