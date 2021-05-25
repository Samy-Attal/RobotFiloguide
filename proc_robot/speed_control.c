#include "LPC23xx.h"

#define FCLK 12000000
#define FREQ_T0 1429    // Hz

#define V_INIT_D 70
#define V_INIT_G 70

void initPWM(){
    PWM1MR0 = 100;                  // "100%"
    PWM1MR1 = V_INIT_D;             // "XX%"
    PWM1MR2 = V_INIT_G;             // "XX%"
    PWM1PCR = (1<<9)|(1<<10);       // PWM 1.1 et 1.2 single edge
    PINSEL3 = (1<<5)|(1<<9);        // 1.18 & 1.20 en PWM
    PWM1MCR = 0x2;                  // reset match MR0
    PWM1TCR = 0x9;                  // activation compteur & PWM
    PWM1LER = 0x3F;             
}

void isrT0()__irq{
    
}

void initT0(){
    T0MR0 = FCLK/
}

int main(void){

}