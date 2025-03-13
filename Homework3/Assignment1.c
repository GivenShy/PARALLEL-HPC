#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <immintrin.h>
#include <time.h>
#include <float.h>

#define N 100000000

float iter_min(float* arr){
    float min = arr[0];
    for(int i = 1;i<N;i++){
        if(arr[i]<min){
            min = arr[i];
        }
    }
    return min;
}

float simd_min(float *arr) {
    float min = FLT_MAX;
    __m256 min_vec = _mm256_set1_ps(min);
    
    int i;
    for (i = 0; i <= N - 8; i += 8) {
        __m256 vec = _mm256_loadu_ps(&arr[i]);
        min_vec = _mm256_min_ps(min_vec, vec);
    }
    
    float min_vals[8];
    _mm256_storeu_ps(min_vals, min_vec);


    for (int j = 0; j < 8; j++) {
        if (min_vals[j] < min) min = min_vals[j];
    }
    
    // Process remaining elements
    for (; i < N; i++) {
        if (arr[i] < min) min = arr[i];
    }

    return min;
}
int main(){
    clock_t start,end;
    float* arr = (float*)malloc(N*sizeof(float));
    srand(time(NULL));
    float min = 0.0;
    float max = 100.0;
    for(int i = 0;i<N;i++){
        arr[i] = min + ((float)rand() / RAND_MAX) * (max - min);;
    }

    float min1 = 0;
    float min2 = 0;

    start = clock();
    min1 = iter_min(arr); 
    end = clock();

    double time_used = ((double) (end-start));

    printf("Min is %f, time is %f\n",min1,time_used);

    start = clock();
    min2 = simd_min(arr); 
    end = clock();

    time_used = ((double) (end-start));

    printf("Min is %f, time is %f",min2,time_used);
    free(arr);
    return 0;
}