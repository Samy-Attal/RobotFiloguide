/*
Projet Robot Filoguidé 
Module telemètre ultrason 

roles des timers :
        - Timer 1 : permet de generer les impulsions du TRIG
        - Timer 2 : compte une seconde afin de relancer les mesures
        - Timer 3 : s'interrompt lors de la reception de l'echo et caclcule la distance
        
Branchements sur LPC2368 :    
    LED_OBSTACLE :      P2.9    (GPIO)
    TRIG_TELEM :        P2.8    (GPIO)
    ECHO_TELEM :        P2.11   (EXTINT)
    Switch0 :           P2.6    (GPIO)
    Switch1 :           P2.7    (GPIO)
*/ 

#include "LPC23xx.h"

#define FCLK 12000000
#define FULTRASON 40000

#define DISTANCE_MAX 272                // Hz => (2.5 m) : 1/((2.5/340)/2) 

#define DISTANCE_MIN 13600              // Hz => (5 cm)  

#define DISTANCE_OBSTACLE 0.15          // en m
#define FOBSTACLE 1/(DISTANCE_OBSTACLE/340)

#define VITESSE_SON 340

#define SW_CONFIG0 FIO2PIN&(1<<6)      // 2.6
#define SW_CONFIG1 FIO2PIN&(1<<7)      // 2.7  
#define TRIG_TELEM FIO2PIN&(1<<8)      // 2.8   

int distance = 0;                      // cm 
char obstacle = 0;

int cpt_mes = 0;
char stop_mes = 0;

char echo = 0;

void initGPIO(){
    FIO2DIR |= (1<<8) | (1<<9);         //TRIG TELEM OUT 
    FIO2CLR |= (1<<8) | (1<<9);         // clear
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
    FIO2PIN ^= 1 << 8;
    if(TRIG_TELEM)
        cpt_mes++;
    if(cpt_mes == 1){
        T3TCR = 1;
        echo = 0;
    }
    if(cpt_mes == nb_mes){
        cpt_mes = 0;
        stop_mes = 1;
        T1TCR = 0;
        FIO2CLR |= 1 << 8;
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

// duree entre emission et echo
void isrDuree()__irq{
    if(!echo){
        obstacle = 0;
        FIO2CLR |= 1 << 9;
    }
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
    if(!stop_mes){
        T3TCR = 0;              // stop timer 3
        echo = 1;
        if(T3TC < FCLK/FOBSTACLE){      // < a 15 cm 
            obstacle = 1;
            FIO2SET |= 1 << 9;
        } else {
            obstacle = 0;
            FIO2CLR |= 1 << 9;
        }
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