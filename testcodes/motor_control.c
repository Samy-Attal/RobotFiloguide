/* ver 24 05 2021 
Projet Robot Filoguidé 
Processeur ROBOT
Programme de test du robot incluant la gestion des capteurs (bobines) 
ainsi que le controle des moteurs 

Branchements sur LPC2368 : 
        - fil d'arret coup de poing :   P2.10 (EXT INT 0)
        - bobine droite :               P2.11 (EXT INT 1)
        - bobine gauche :               P2.12 (EXT INT 2)
        - led droite :                  P2.0  (GPIO)
        - led gauche :                  P2.1  (GPIO)
        - moteur droit :                P1.18 (PWM)
        - moteur gauche :               P1.20 (PWM)
*/


#include "LPC23xx.h"

#define FCLK 12000000   // Hz
#define FREQ 1          // Hz

#define V_INIT_D 60           
#define V_INIT_G 80     //valeurs vitesses arbitraires en %

#define VMAX 80
#define VMIN 25


void speedSwap(){
    int tmp;
    if(((FIO2PIN&0x1)&&(PWM1MR1>PWM1MR2))||((FIO2PIN&0x2)&&(PWM1MR2 > PWM1MR1))){
        tmp = PWM1MR1;
        PWM1MR1 = PWM1MR2;
        PWM1MR2 = tmp;
    }
    PWM1LER = 0x3F;             
}

void initGPIO(){
    PINSEL4 = 0;                  // GPIO
    FIO2DIR = 0x3;
}

// arret coup de poing
void isrArret()__irq{
    PWM1MR1 = 0;    
    PWM1MR2 = 0;
    PWM1LER = 0x3F;
    EXTINT = 0x1;
    VICVectAddr = 0;
}

void initArret(){
    PINSEL4 |= 1 << 20;             // 2.10 en EINT0
    EXTMODE |= 0x1;                 // front 
    EXTPOLAR |= 0x0;                // descendant (fil arraché)
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

void isrBobineGauche()__irq{
	FIO2PIN = 0x2;			        // alume led gauche
    T0TCR = 1;			        	// activation timer pour allumer la led 1 seconde
    speedSwap();
	VICVectAddr = 0;
	EXTINT = 0x4;
}

void initBobineGauche(){			
	PINSEL4 |= 1 << 24;             // 2.12 EXT INT 2
	EXTMODE |= 0x4;                 // front 
	EXTPOLAR |= 0x4;                // montant

	VICIntEnable = 1 << 16;
	VICVectAddr16 = (unsigned long)isrBobineGauche;
}

void isrBobineDroite()__irq{			
	FIO2PIN = 0x1;			        // allume led droite
	T0TCR = 1;				        // activation timer pour allumer la led 1 seconde
    speedSwap();
	VICVectAddr = 0;
	EXTINT = 0x2;
}

void initBobineDroite(){			 
	PINSEL4 |= 1 << 22;             // EXT INT 1	
	EXTMODE |= 0x2;			        // front
	EXTPOLAR |= 0x2;		        // montant
	
	VICIntEnable = 1 << 15;
	VICVectAddr15 = (unsigned long)isrBobineDroite;
}

void isrT0()__irq{
	FIO2PIN = 0;			        // eteint les leds
	T0IR = 1;
	VICVectAddr = 0;
}

void initT0(){
	T0MR0 = FCLK/FREQ;		        // compte 1 seconde 
	T0MCR = 0x7;			        // reset + interrupt + stop
	VICIntEnable = 1 << 4;
	VICVectAddr4 = (unsigned long)isrT0;
}

void initALL(){
    initGPIO();
    initArret();
    initBobineDroite();
    initBobineGauche();
    initT0();
    initPWM();
}

int main(void){
    initALL();
    while(1);
}