#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define P  3
#define R  1000

int wins[P] = {0};
int rolls[P] = {0};

pthread_barrier_t roll_barrier;

void print_winner(){
    int win = -1;
    int winner = -1;
    int winnerExist = 1;
    for(int i = 0;i<P;i++){
        printf("Player %d: %d\n",i,wins[i]);
        if(win<wins[i]){
            winner = i;
            win = wins[i];
        }
        if(win=wins[i]){
            winnerExist = 0;
        }
    }
    if(winnerExist = 1)
    {
        printf("Winner is Player No: %d\n",winner);
    }
    else{
        printf("There is no winner");
    }
}

void determine_winner(){
    int roll = -1;
    int winner = -1;
    int winnerExist = 1;
    for(int i = 0;i<P;i++){
        if(roll<rolls[i]){
            winner = i;
            roll = rolls[i];
            winnerExist = 1;
        }
        if(roll == rolls[i]){
            winnerExist = 0;
        }
    }
    if(winnerExist = 1){
        wins[winner]++;
    }
}

void* roll_dice(void* arg){
    int *args = (int*)arg;
    srand(time(0)*47+(args[0]+1)*(args[1]+1)*13);
    int random = rand()%6 +1;
    printf("Random:%d, Player:%d\n",random,args[0]);
    
    rolls[args[0]] = random;
    free(args);
    pthread_barrier_wait(&roll_barrier);
}

int main() 
{
    pthread_t thread[P];
    
    pthread_barrier_init(&roll_barrier, NULL, P+1);
    for (int i = 0; i<R;i++){
        for(int j = 0; j<P;j++){
            int *args = malloc(2*sizeof(int));
            args[0] = j;
            args[1] = i;
            pthread_create(&thread[j],NULL,roll_dice, args);
        }
        pthread_barrier_wait(&roll_barrier);
        determine_winner();
    }
    print_winner();
    pthread_barrier_destroy(&roll_barrier);

    return 0;
}