/*
Author: Adam Handke
Task: Producer–consumer synchronization problem of producing water particles
SOlved on threads with the use of mutexes and conditional variables.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

unsigned int ILE_H2O;        //ile czasteczek wody ma powstac
unsigned int ILE_PROD_H;      //ile producentow wodoru
unsigned int ILE_PROD_O;      //ile producentow tlenu

unsigned int liczba_wodorow = 0;
unsigned int liczba_tlenow = 0;

pthread_mutex_t mutex_wodor = PTHREAD_MUTEX_INITIALIZER; //mutex od liczby wodorow
pthread_mutex_t mutex_tlen = PTHREAD_MUTEX_INITIALIZER;  //mutex od liczby tlenow
pthread_cond_t cond_wodor_oddali = PTHREAD_COND_INITIALIZER; //zmienna warunkowa dla producentow wodoru
pthread_cond_t cond_tlen_oddali = PTHREAD_COND_INITIALIZER;    //zmienna warunkowa dla producentow tlenu

void *prod_wodoru(void* p){
    unsigned int buf = 0;
    unsigned int wyprodukowano = 0;

    unsigned int seed, random_wait;
    seed = time(NULL) ^ getpid() ^ pthread_self();
    random_wait = rand_r(&seed)%1000000; //losowe mikrosekundy, max 1 sekunda

    while(wyprodukowano < (2*ILE_H2O)/ILE_PROD_H){
        printf("H_%d\t zaczyna produkcje wodoru\n", (int)pthread_self()%1000);
        //produkcja do bufora w wylosowanym czasie
        usleep(random_wait);
        buf++;
        wyprodukowano++;
        //printf("H_%d\t wyprodukowal wodor nr %d\n", pthread_self()%1000, wyprodukowano);

        //oddawanie wyprodukowanego atomu na wode
        pthread_mutex_lock(&mutex_wodor);
        liczba_wodorow++;
        printf("H_%d\t ma swoj %d wodor i czeka\n", (int)pthread_self()%1000, wyprodukowano);
        pthread_cond_wait(&cond_wodor_oddali, &mutex_wodor);
        printf("H_%d\t oddal wodor\n", (int)pthread_self()%1000);
        buf--;
        pthread_mutex_unlock(&mutex_wodor);
    }
    pthread_exit(0);
}

void *prod_tlenu(void* p){
    unsigned int buf = 0;
    unsigned int wyprodukowano = 0;

    unsigned int seed, random_wait;
    seed = time(NULL) ^ getpid() ^ pthread_self();
    random_wait = rand_r(&seed)%1000000; //losowe mikrosekundy, max 1 sekunda

    while(wyprodukowano < ILE_H2O/ILE_PROD_O){
        printf("O_%d\t zaczyna produkcje tlenu\n", (int)pthread_self()%1000);
        //produkcja do bufora w wylosowanym czasie
        usleep(random_wait);
        buf++;
        wyprodukowano++;
        //printf("O_%d\t wyprodukowal tlen nr %d\n", pthread_self()%1000, wyprodukowano);

        //oddawanie wyprodukowanego atomu na wode
        pthread_mutex_lock(&mutex_tlen);
        liczba_tlenow++;
        printf("O_%d\t ma swoj %d tlen i czeka\n", (int)pthread_self()%1000, wyprodukowano);
        pthread_cond_wait(&cond_tlen_oddali, &mutex_tlen);
        printf("O_%d\t oddal tlen\n", (int)pthread_self()%1000);
        buf--;
        pthread_mutex_unlock(&mutex_tlen);
    }
    pthread_exit(0);
}

void *kons_wody(void* p){

    unsigned int woda_gotowa = 0;

    while(woda_gotowa < ILE_H2O){
        //wejscie do podwojnie krytycznej sekcji ze sprawdzeniem warunkow
        pthread_mutex_lock(&mutex_wodor);
        pthread_mutex_lock(&mutex_tlen);
        if(liczba_wodorow >= 2 && liczba_tlenow >= 1){
            //jesli jest wystarczajaco atomow, to wytwarza wode
            woda_gotowa++;
            liczba_wodorow-=2;
            liczba_tlenow--;
            printf("H2O_%d\t wytworzyl wode nr %d\n\n", (int)pthread_self()%1000, woda_gotowa);
            pthread_cond_signal(&cond_wodor_oddali);
            pthread_cond_signal(&cond_wodor_oddali);    //dwa wodory pobrano
            pthread_cond_signal(&cond_tlen_oddali);     //jeden tlen pobrano
        }
        pthread_mutex_unlock(&mutex_tlen);
        pthread_mutex_unlock(&mutex_wodor);
    }

    usleep(500000);
    printf("H2O_%d\t wytworzyl %d czasteczek wody\n", (int)pthread_self()%1000, woda_gotowa);
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("Blad! Nalezy podac 3 argumenty: [ile H2O] [ile prod. H] [ile prod. O]\n");
        return 0;
    }
    ILE_H2O = atoi(argv[1]);
    ILE_PROD_H = atoi(argv[2]);
    ILE_PROD_O = atoi(argv[3]);

    if(ILE_H2O*2 % ILE_PROD_H != 0 || ILE_H2O % ILE_PROD_O != 0){
        printf("Blad! Nieprawidlowe argumenty - liczby potrzebnych H i O musza dzielic sie rowno na producentow.\n");
        return 0;
    }

    int i;

    pthread_t wodor[ILE_PROD_H];
    pthread_t tlen[ILE_PROD_O];
    pthread_t woda;

    //create-owanie watkow
    for(i=0; i<ILE_PROD_H; i++){
        pthread_create(&wodor[i], NULL, prod_wodoru, NULL);
    }
    for(i=0; i<ILE_PROD_O; i++){
        pthread_create(&tlen[i], NULL, prod_tlenu, NULL);
    }
    pthread_create(&woda, NULL, kons_wody, NULL);

    //join-owanie watkow
    for(i=0; i<ILE_PROD_H; i++){
        pthread_join(wodor[i], NULL);
    }
    for(i=0; i<ILE_PROD_O; i++){
        pthread_join(tlen[i], NULL);
    }
    pthread_join(woda, NULL);

    pthread_cond_destroy(&cond_wodor_oddali);
    pthread_cond_destroy(&cond_tlen_oddali);

    return 0;
}
