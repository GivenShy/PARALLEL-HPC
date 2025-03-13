#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <immintrin.h>
#include <time.h>
#include <float.h>

#define N 2000000

void iter_prefix(float* input, float* output){
    output[0] = input[0];
    for(int i = 1; i<N;i++){
        output[i] = output[i-1] + input[i];
    }
}
void simd_prefix(float* input, float* output){
    __m256 temp = _mm256_setzero_ps();
    int i;
    for(i = 0; i < N-8; i+=8){

        __m256 current = _mm256_loadu_ps(&input[i]);
        current = _mm256_add_ps(temp,current);
        __m256 shifted = _mm256_loadu_ps(&input[i]);
        __m256i indices = _mm256_setr_epi32(-1, 0, 1, 2, 3, 4, 5, 6); 
        shifted = _mm256_permutevar8x32_ps(shifted, indices);

        shifted = _mm256_blend_ps(shifted, _mm256_setzero_ps(), 1);
        for(int j = 0; j < 7; j++){
            current = _mm256_add_ps(current,shifted);
        
            shifted = _mm256_permutevar8x32_ps(shifted, indices);
            shifted = _mm256_blend_ps(shifted, _mm256_setzero_ps(), 1);
        }

        _mm256_storeu_ps(&output[i],current);
        temp = _mm256_set1_ps(output[i+7]);
    }
    for(;i<N;i++){
        output[i] = output[i-1] + input[i];
    }
}

int main(){
    clock_t start,end;
    float* input = (float*)malloc(N*sizeof(float));
    float* output = (float*)malloc(N*sizeof(float));
    float min = 0.0;
    float max = 100.0;
    for(int i = 0;i<N;i++){
        input[i] = min + ((float)rand() / RAND_MAX) * (max - min);;
    }

    start = clock();
    iter_prefix(input,output);

    end = clock();

    double time_used = ((double) (end-start));
    printf("Iter: Time is %f\n",time_used);

    start = clock();
    simd_prefix(input,output); 
    end = clock();

    time_used = ((double) (end-start));

    printf("SIMD: Time is %f\n",time_used);

    return 0;
}