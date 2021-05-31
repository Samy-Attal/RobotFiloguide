/*
Projet Robot Filoguidé version "bourré" FONCTIONNELLE
controle des moteurs en fonction des capteurs bobines
interverti la vitesse des moteurs en fonction de sa position 
Branchements sur le LPC2368
        - arret coup de poing :             P2.10   (EXTINT)
        - bp mise en marche :               P2.2    (GPIO)
        - dephasage avant :                 P2.0    (GPIO)
        - dephasage arriere :               P2.1    (GPIO)
        - Amplitude crete bobine avant :    P0.23   (ADC)
        - commande moteur droit :           P1.18   (PWM)
        - commande moteur gauche :          P1.20   (PWM)
        - bp gare :                         P1.0    (GPIO)
        - bp reprise :                      P1.1    (GPIO)
*/

#include "LPC23xx.h"

#define FCLK 12000000   // Hz
#define FREQ 1          // Hz
#define FREQ_T0 1429    // Hz

#define V_INIT_D 70           
#define V_INIT_G 80     //valeurs vitesses arbitraires en %

#define V_MAX 80
#define V_MIN 60

#define ADC_MAX 0x3A2   // 3.3V

#define ADC_MIN 0x136   // 1.0V

#define XOR_AVANT FIO2PIN&0x1
#define XOR_ARRIERE FIO2PIN&0x2

#define BP_GARE FIO1PIN&0x1
#define BP_REPRISE FIO1PIN&0x2

#define RED FIO1PIN&0x4
#define GREEN FIO1PIN&0x8
#define BLUE FIO1PIN&0x10

#define V_MOT_D PWM1MR1
#define V_MOT_G PWM1MR2

#define CONTINUE FIO2PIN&0x4

char stop = 0;

char right = 0;
char left = 0;

volatile unsigned int ADC0 = 0;
volatile unsigned int ADC1 = 0;
volatile unsigned int ADC2 = 0;

/*
volatile int flag_gare = 0;
volatile int flag_reprise = 0;
*/

char cpt_1e = 0;
char cpt_2e = 0;
int cpt1 = 100;
int cpt2 = 100;
int etat = 0;
char attente = 0;

void setRGB(int state){
    if(state == 0)
        FIO1PIN = 0x8;              // vert
    else if(state == 1 || state == 2)
        FIO1PIN = 0x4;              // red
    else if(state == 3)
        FIO1PIN = 0x10 | 0x4;       // violet
}

void stopMotors(int state){
    if(state == 3){
        PWM1MR1 = 0;
        PWM1MR2 = 0;
    }
    PWM1LER = 0x3F;
}

void setMotors(int state){
    if(state == 1){
        V_MOT_D = 70;
        V_MOT_G = 0;
    }
    else if(state == 2){
        V_MOT_D = 0;
        V_MOT_G = 70;
    }
    PWM1LER = 0x3F;
}
void initGPIO(){
    FIO1DIR = 0x1C;     // P1. 2 3 4 
    FIO1CLR = 0x1C;
}


void speedSwap(char r, char l){
    if(r){
	    V_MOT_D = V_MAX;
		V_MOT_G = V_MIN;
	}
	else if(l){
		V_MOT_D = V_MIN;
        V_MOT_G = V_MAX;
	}
	PWM1LER = 0x3F;
}

// arret coup de poing
void isrArret()__irq{
    stop = 1;
    V_MOT_D = 0;    
    V_MOT_G = 0;
    PWM1LER = 0x3F;
    EXTINT = 0x1;
    VICVectAddr = 0;
}

void initArret(){
    PINSEL4 |= 1 << 20;             // 2.10 en EINT0
    EXTMODE |= 0x1;                 // front 
    EXTPOLAR |= 0x0;                // descendant (fil arrachÃ©)
    VICIntEnable = 1 << 14;
    VICVectAddr14 = (unsigned long)isrArret;
}

// PWM Moteurs
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

void isrADC()__irq{
    AD0CR &= ~(1<<16);              // arret de la conversion
    ADC0 = (AD0DR0>>6)&0x3FF;
    ADC1 = (AD0DR1>>6)&0x3FF;
    ADC2 = (AD0DR2>>6)&0x3FF;
    VICVectAddr = 0;
}

void initADC(){
    PCONP |= 1 << 12;                           // alimentation ADC
    AD0CR |= 1 << 21;                           // PDN
    PINSEL1 |= (0x1<<14)|(0x1<<16)|(0x1<<18);   // ADC
    PINMODE1 |= (0x2<<14)|(0x2<<16)|(0x2<<18);  // no pull up or down 
    AD0CR |= 2 << 8;
    AD0INTEN = 0x4;
    AD0CR |= 0x7;
    VICIntEnable = 1 << 18;
    VICVectAddr18 = (unsigned long)isrADC;
}

void isrT0()__irq{
    AD0CR |= 1 << 16;
    if(BP_GARE)
        etat = 1;
    if(cpt_1e)
        cpt1--;
    if(!cpt1){
        cpt1 = 100;
        etat = 2;
    }
    if(cpt_2e)
        cpt2--;
    if(!cpt2){
        cpt2 = 100;
        etat = 3;
    }
    if(attente){
        if(BP_REPRISE){
            etat = 0;
            attente = 0;
        }
    }
    if(!etat){
        if(XOR_AVANT && !(XOR_ARRIERE)){
            right = 1;
            left = 0;
        }
        else if(!(XOR_AVANT) && XOR_ARRIERE){
            right = 0;
            left = 1;
        }
        else{
            right = 0;
            left = 0;
        }

        if(CONTINUE)
            stop = 0;
        if(!stop)
            speedSwap(right, left);
            //speedAdapt(right, left);
    }
    T0IR = 0x3F;
    VICVectAddr = 0;
}

void initT0(){
    T0MR0 = 12000;                          // FCLK/FREQ_T1;
	T0MCR = 0x3;                            // reset + interrupt
    T0IR = 0x1;			
    VICVectAddr4 = (unsigned long)isrT0;        
	VICIntEnable = 1 << 4;
    T0TCR = 0x1;
}

void initLPC(){
    SCS = 1;
	PINMODE4 |= 0x3F;   // pull down resistors on 2.0 2.1 and 2.2
    initGPIO();
    initArret();
    initADC();
    initT0();
    initPWM();
}

int main(void){
    initLPC();
    while(1){
        switch(etat){
            case 0 : attente = 0; cpt_1e = 0; cpt_2e = 0; break;
            case 1 : attente = 0; cpt_1e = 1; cpt_2e = 0; break;
            case 2 : attente = 0; cpt_1e = 0; cpt_2e = 1; break;
            case 3 : attente = 1; cpt_1e = 0; cpt_2e = 0; break;
            default : attente = 0; cpt_1e = 0; cpt_2e = 0;
        }
        setMotors(etat);
        stopMotors(etat);
        setLeds(etat);
    }
}