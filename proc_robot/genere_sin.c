#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14
#define SIZE 100
#define FREQ 1000

#define SINEFILE "son.txt"

unsigned int* createSine(unsigned int freq, unsigned int ech){
    unsigned int i, *tab;
    tab = calloc(ech,sizeof(int));
    for(i=0;i<ech;i++){
        tab[i] = 512*sin(2*PI*freq*i)+512;
    }
    return tab;
}

void afficheTab(unsigned int* tab, unsigned int size){
    int i;
    for(i=0;i<size;i++){
        printf("%d ", tab[i]);
    }
}

void writeSine(unsigned int* tab, int size, char* name){
    FILE* file = fopen(name, "w");
    int i;
    for(i=0;i<size;i++){
        fprintf(file, "%d,",tab[i]);
    }
    fclose(file);
}

int main(){
    unsigned int* sinus; 
    sinus = createSine(FREQ, SIZE);
    //afficheTab(sinus, SIZE);
    writeSine(sinus, SIZE, SINEFILE);
    return 0;
}