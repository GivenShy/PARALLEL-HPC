#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define M 10

pthread_barrier_t barrier;
pthread_barrier_t endBarrier;


void* startPipline(void* args){
    int *threadId = (int*)args; 
    printf("Thread %d started pipeline 1\n", *threadId);
    srand(time(NULL)*3+ (*threadId+1)*17);
    long rand_time = rand()%5+1;
    sleep(rand_time);
    printf("Thread %d finished pipeline 1\n", *threadId);
    pthread_barrier_wait(&barrier);
    printf("Thread %d started pipeline 2\n", *threadId);
    srand(time(NULL)*3+ (*threadId+1)*17);
    rand_time = rand()%5+1;
    sleep(rand_time);
    printf("Thread %d finished pipeline 2\n", *threadId);
    pthread_barrier_wait(&barrier);
    printf("Thread %d started pipeline 3\n", *threadId);
    srand(time(NULL)*3+ (*threadId+1)*17);
    rand_time = rand()%5+1;
    sleep(rand_time);
    printf("Thread %d finished pipeline 3\n", *threadId);
    free(args);
    pthread_barrier_wait(&endBarrier);
}



int main(){
    pthread_t threads[M];
    pthread_barrier_init(&barrier,NULL,M);
    pthread_barrier_init(&endBarrier,NULL,M+1);
    for(int i = 0; i < M; i++){
        int *args = malloc(sizeof(int));
        *args = i;
        pthread_create(&threads[i],NULL, startPipline, args);
    }
    pthread_barrier_wait(&endBarrier);
    printf("All piplelines are finished");
    pthread_barrier_destroy(&barrier);
    pthread_barrier_destroy(&endBarrier);
    return 0;
}