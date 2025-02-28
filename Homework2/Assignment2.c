#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PLAYERS  100

pthread_barrier_t read_barrier;

void* getting_ready(void* arg){
    int *player = (int*)arg;
    srand(time(NULL)*3+ (*player+1)*17);
    long rand_time = rand()%5+1;
    sleep(rand_time);
    srand(time(NULL)*17+ (*player+1)*7);
    rand_time = rand()%8+1;
    sleep(rand_time);
    printf("Player %d is ready\n",*player);
    pthread_barrier_wait(&read_barrier);
}

int main() 
{
    pthread_t thread[PLAYERS];
    pthread_barrier_init(&read_barrier, NULL, PLAYERS+1);
    for (int i = 0;i< PLAYERS; i++){
        int *args = malloc(sizeof(int));
        *args = i;
        pthread_create(&thread[i],NULL,getting_ready, args);
    }
    pthread_barrier_wait(&read_barrier);
    printf("Game Started!");
    return 0;
}