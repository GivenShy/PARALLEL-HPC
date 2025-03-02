#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SENSORS 10

pthread_barrier_t barrier;

int data[SENSORS];

void* getSensorData(void* args){
    int *sensorIndex = (int*)args; 
    srand(time(NULL)*3+ (*sensorIndex+1)*17);
    long rand_time = rand()%5+1;
    sleep(rand_time);
    srand(time(0)*17+(*sensorIndex+1)*13);
    data[*sensorIndex] = rand()%100+1;
    free(args);
    pthread_barrier_wait(&barrier);
}

double processData(){
    pthread_barrier_wait(&barrier);
    int sum = 0;
    for(int i = 0; i< SENSORS; i++){
        sum+=data[i];
    }
    return (double)sum/SENSORS;
}

int main(){
    pthread_t sensors[SENSORS];
    pthread_barrier_init(&barrier,NULL,SENSORS+1);
    for(int i = 0; i < SENSORS; i++){
        int *args = malloc(sizeof(int));
        *args = i;
        pthread_create(&sensors[i],NULL, getSensorData, args);
    }
    double avg = processData();
    printf("Avarage: %f\n",avg);
    pthread_barrier_destroy(&barrier);

    return 0;
}