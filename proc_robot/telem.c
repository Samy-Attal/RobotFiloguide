/*
Projet Robot Filoguidé 
Module telemètre ultrason 

Branchements sur LPC2368 :
    TRIG_TELEM :        P3.0    (GPIO)
    LED_OBSTACLE :      P3.1    (GPIO)
    ECHO_TELEM :        P2.11   (EXTINT)
*/


#include "LPC23xx.h"

#define FCLK 12000000
#define FULTRASON 40000

#define DISTANCE_MAX 272           // Hz => (2.5 m) : 1/((2.5/340)/2) 
    
#define VITESSE_SON 340

#define SW_CONFIG0 FIO2PIN&0x4      // 2.2
#define SW_CONFIG1 FIO2PIN&0x8      // 2.3  
#define TRIG_TELEM FIO1PIN&(1<<19)  

int distance = 0;             // cm 
char obstacle = 0;

int cpt_mes = 0;
char stop_mes = 0;

void initGPIO(){
    PINSEL6 |= 0x0;             // 3.0 et 3.1 en GPIO
    FIO3DIR |= 0x3;             // OUT 
    FIO3CLR |= 0x3;             // clear
}
char switchesConfig(char sw0, char sw1){
    char nb;
    if(!sw0 && !sw1)
        nb = 10;
    else if(sw0 && !sw1)
        nb = 15;
    else if(!sw0 && sw1)
        nb = 20;
    else 
        nb = 25;
    return nb;
}

void isrTrig()__irq{
    char nb_mes = switchesConfig(SW_CONFIG0, SW_CONFIG1);
    FIO3PIN ^= 0x1;
    if(FIO3PIN&0x1)
        cpt_mes++;
    if(cpt_mes == 1)
        T3TCR = 1;
    if(cpt_mes == nb_mes){
        cpt_mes = 0;
        stop_mes = 1;
        T1TCR = 0;
        FIO3CLR |= 0x1;
    }

    T1IR = 0x1;
    VICVectAddr = 0; 
}

void initTrig(){
    T1MR0 = FCLK/FULTRASON/2;
    T1MCR = 0x3;                 // reset + interrupt
    VICIntEnable = 1 << 5;
    VICVectAddr5 = (unsigned long)isrTrig;
    T1TCR = 1;
}

void isrT2()__irq{
    T1TCR = 1;
    stop_mes = 0;
    T2IR = 1;
    VICVectAddr = 0;
}

void initT2(){
    T2MR0 = FCLK;               // compte 1 seconde
    T2MCR = 0x3;                // reset + interrupt
    VICIntEnable = 1 << 26;
    VICVectAddr26 = (unsigned long)isrT2;
    T2TCR = 1;
}

void isrDuree()__irq{
    obstacle = 0;
    FIO3CLR |= 0x2;
    distance = 250;             // voire + echo non recu
    T3IR = 1;
    VICVectAddr = 0;
}

void initDuree(){
    T3MR0 = FCLK/DISTANCE_MAX;
    T3MCR = 0x7;                // reset + interrupt + stop
    VICIntEnable = 1 << 27;
    VICVectAddr27 = (unsigned long)isrDuree;
}

void isrEcho()__irq{
    T3TCR = 0;              // stop timer 3
    distance = 100*(VITESSE_SON*T3TC)/2;
    if(distance < 15){      // < a 15 cm 
        obstacle = 1;
        FIO3SET |= 0x2;
    } else {
        obstacle = 0;
        FIO3CLR |= 0x2;
    }
    EXTINT = 0x2;
    VICVectAddr = 0;
}

void initEcho(){
    PINSEL4 = 1 << 22;      // P2.11 en EINT1
    EXTMODE = 0x2;
    EXTPOLAR = 0x2;
    VICIntEnable = 1 << 15;
    VICVectAddr15 = (unsigned long)isrEcho;
}

void initLPC(){
    initGPIO();
    initTrig();
    initEcho();
    initT2();
    initDuree();
}

int main(void){
    initLPC();
    while(1);
}