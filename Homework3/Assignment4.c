#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <immintrin.h>
#include <time.h>
#include <float.h>

#define M  2000
#define N  2000

void iter_mult(float A[M][N], float x[N], float y[M]){
    for(int i = 0; i < M; i++){
        y[i] = 0.0;
        for(int j = 0;j<N;j++){
            y[i] += A[i][j]*x[j];
        }
    }
}

void simd_mult(float A[M][N], float x[N], float y[M]){
    for(int i = 0; i< M;i++){
        __m256 sum = _mm256_setzero_ps();
        int j;
        for(j = 0;j<N - 8;j+=8){
            __m256 a_mat = _mm256_loadu_ps(&A[i][j]);
            __m256 x_vec = _mm256_loadu_ps(&x[j]);

            __m256 temp = _mm256_mul_ps(a_mat,x_vec);
            sum = _mm256_add_ps(sum,temp);
        }
        float s[8];
        _mm256_storeu_ps(s, sum);
        y[i] = s[0] + s[1] + s[2] + s[3] + s[4] + s[5] + s[6] + s[7];
        for(;j<N;j++){
            y[i] += A[i][j]*x[j];
        }
    }
}

int main(){
    float A[M][N], x[N], y_simd[M];
    srand(time(NULL));

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            A[i][j] = (float)(rand())/3;

    for (int j = 0; j < N; j++)
        x[j] = (float)(rand())/3;

    clock_t start, end;

    // Run SIMD version
    start = clock();
    iter_mult(A, x, y_simd);
    end = clock();
    double time_simd = ((double)(end - start));

    // Print execution time
    printf("Iter Execution Time: %f sec\n", time_simd);

    start = clock();
    simd_mult(A, x, y_simd);
    end = clock();
    time_simd = ((double)(end - start));

    // Print execution time
    printf("SIMD Execution Time: %f sec\n", time_simd);

    return 0;
}