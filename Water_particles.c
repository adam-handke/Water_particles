/*
Author: Adam Handke
Task: Producer–consumer synchronization problem of producing water particles
Solved on threads with the use of mutexes and conditional variables.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

unsigned int ILE_H2O;        //how many H2O particles
unsigned int ILE_PROD_H;      //how many hydrogen producers
unsigned int ILE_PROD_O;      //how many oxygen producers

unsigned int liczba_wodorow = 0;
unsigned int liczba_tlenow = 0;

pthread_mutex_t mutex_wodor = PTHREAD_MUTEX_INITIALIZER; //hydrogen mutex
pthread_mutex_t mutex_tlen = PTHREAD_MUTEX_INITIALIZER;  //oxygen mutex
pthread_cond_t cond_wodor_oddali = PTHREAD_COND_INITIALIZER; //conditional variable for hydrogen producers
pthread_cond_t cond_tlen_oddali = PTHREAD_COND_INITIALIZER;    //conditional variable for oxygenproducers

//hydrogen producer
void *prod_wodoru(void* p){
    unsigned int buf = 0;
    unsigned int wyprodukowano = 0;

    unsigned int seed, random_wait;
    seed = time(NULL) ^ getpid() ^ pthread_self();
    random_wait = rand_r(&seed)%1000000; //random microseconds, max 1 second

    while(wyprodukowano < (2*ILE_H2O)/ILE_PROD_H){
        printf("H_%d\t begins hydrogen production\n", (int)pthread_self()%1000);
        //production to buffer in random time
        usleep(random_wait);
        buf++;
        wyprodukowano++;
        //printf("H_%d\t wyprodukowal wodor nr %d\n", pthread_self()%1000, wyprodukowano);

        //giving the hydrogen atom for water
        pthread_mutex_lock(&mutex_wodor);
        liczba_wodorow++;
        printf("H_%d\t has its %d hydrogen and wiats\n", (int)pthread_self()%1000, wyprodukowano);
        pthread_cond_wait(&cond_wodor_oddali, &mutex_wodor);
        printf("H_%d\t has given hydrogen\n", (int)pthread_self()%1000);
        buf--;
        pthread_mutex_unlock(&mutex_wodor);
    }
    pthread_exit(0);
}

//oxygen producer
void *prod_tlenu(void* p){
    unsigned int buf = 0;
    unsigned int wyprodukowano = 0;

    unsigned int seed, random_wait;
    seed = time(NULL) ^ getpid() ^ pthread_self();
    random_wait = rand_r(&seed)%1000000; //random microseconds, max 1 second

    while(wyprodukowano < ILE_H2O/ILE_PROD_O){
        printf("O_%d\t begins oxygen production\n", (int)pthread_self()%1000);
        //production to buffer in random time
        usleep(random_wait);
        buf++;
        wyprodukowano++;
        //printf("O_%d\t wyprodukowal tlen nr %d\n", pthread_self()%1000, wyprodukowano);

        //giving the oxygen atom for water
        pthread_mutex_lock(&mutex_tlen);
        liczba_tlenow++;
        printf("O_%d\t has its %d oxygen and waits\n", (int)pthread_self()%1000, wyprodukowano);
        pthread_cond_wait(&cond_tlen_oddali, &mutex_tlen);
        printf("O_%d\t has give oxygen\n", (int)pthread_self()%1000);
        buf--;
        pthread_mutex_unlock(&mutex_tlen);
    }
    pthread_exit(0);
}

//water consumer
void *kons_wody(void* p){

    unsigned int woda_gotowa = 0;

    while(woda_gotowa < ILE_H2O){
        //enterance to a double critical conditional section
        pthread_mutex_lock(&mutex_wodor);
        pthread_mutex_lock(&mutex_tlen);
        if(liczba_wodorow >= 2 && liczba_tlenow >= 1){
            //if has enough atoms, then make water H2O
            woda_gotowa++;
            liczba_wodorow-=2;
            liczba_tlenow--;
            printf("H2O_%d\t wytworzyl wode nr %d\n\n", (int)pthread_self()%1000, woda_gotowa);
            pthread_cond_signal(&cond_wodor_oddali);
            pthread_cond_signal(&cond_wodor_oddali);    //used 2 hydrogens
            pthread_cond_signal(&cond_tlen_oddali);     //used 1 oxygen
        }
        pthread_mutex_unlock(&mutex_tlen);
        pthread_mutex_unlock(&mutex_wodor);
    }

    usleep(500000);
    printf("H2O_%d\t created %d water particles\n", (int)pthread_self()%1000, woda_gotowa);
    pthread_exit(0);
}

int main(int argc, char *argv[]){
    if(argc != 4){
        printf("Error! 3 arguments needed: [how much H2O] [hom many H producers] [how many O producers]\n");
        return 0;
    }
    ILE_H2O = atoi(argv[1]);
    ILE_PROD_H = atoi(argv[2]);
    ILE_PROD_O = atoi(argv[3]);

    if(ILE_H2O*2 % ILE_PROD_H != 0 || ILE_H2O % ILE_PROD_O != 0){
        printf("Error! Wrong arguments - the numbers of needed H and O must divide equally among producers.\n");
        return 0;
    }

    int i;

    pthread_t wodor[ILE_PROD_H];
    pthread_t tlen[ILE_PROD_O];
    pthread_t woda;

    //creating threads
    for(i=0; i<ILE_PROD_H; i++){
        pthread_create(&wodor[i], NULL, prod_wodoru, NULL);
    }
    for(i=0; i<ILE_PROD_O; i++){
        pthread_create(&tlen[i], NULL, prod_tlenu, NULL);
    }
    pthread_create(&woda, NULL, kons_wody, NULL);

    //joining threads
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
