/*
Robot Filoguidé
Processeur BASE 
Programme de test de l'emission PWM dans le fil
genere un signal carré de frequence 50kHz

Branchements sur LPC2368 : 
        - entrée d'emission RLC : P1.18 (PWM)
*/

#include "LPC23xx.h"

#define FCLK 12000000       // Hz
#define FSIGNAL 50000       // Hz

void initPWM(){
    PWM1MR0 = FCLK/FSIGNAL;
    PWM1MR1 = 0.5*PWM1MR0;
    PWM1PCR = 1 << 9;
    PINSEL3 = 1 << 5;
    PWM1MCR = 0x2;
    PWM1TCR = 0x9;
    PWM1LER = 0x3F;
}

int main(void){
    initPWM();
    while(1);
}