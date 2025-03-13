#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <immintrin.h>
#include <time.h>
#include <float.h>

#define N 20000000
#define W1 0.1
#define W2 0.2
#define W3 0.3

void iter_conv(float* input, float* output){
    for(int i = 1; i<N-1;i++){
        output[i] = input[i-1]*W1 + input[i]*W2 + input[i+1]*W3;
    }
}

void simd_conv(float* input, float* output){
    __m256 w1 = _mm256_set1_ps(W1);
    __m256 w2 = _mm256_set1_ps(W2);
    __m256 w3 = _mm256_set1_ps(W3);

    int i = 1;
    for(i = 1; i<N - 7; i+=8){
        __m256 first = _mm256_loadu_ps(&input[i-1]);
        __m256 second = _mm256_loadu_ps(&input[i]);
        __m256 third = _mm256_loadu_ps(&input[i+1]);

        __m256 f = _mm256_mul_ps(first,w1);
        __m256 s = _mm256_mul_ps(second,w2);
        __m256 t = _mm256_mul_ps(third,w3);

        __m256 sum1 = _mm256_add_ps(f,s);
        __m256 sum = _mm256_add_ps(sum1,t);
        _mm256_storeu_ps(&output[i],sum);

    }
    for(;i<N-1;i++){
        output[i] = input[i-1]*W1 + input[i]*W2 + input[i+1]*W3;
    }
}

int main(){
    clock_t start,end;

    float* input = (float*)malloc(N*sizeof(float));
    float* output = (float*)malloc(N*sizeof(float));
    float min = 0.0;
    float max = 255.0;
    for(int i = 0;i<N;i++){
        input[i] = min + ((float)rand() / RAND_MAX) * (max - min);;
    }
    
    start = clock();
    iter_conv(input,output); 
    end = clock();

    double time_used = ((double) (end-start));

    printf("Iter: Time is %f\n",time_used);

    start = clock();
    simd_conv(input,output); 
    end = clock();

    time_used = ((double) (end-start));

    printf("SIMD: Time is %f\n",time_used);

    free(input);
    free(output);
    return 0;
}