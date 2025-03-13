#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <immintrin.h>
#include <time.h>
#include <float.h>

#define s 10000000

void iter_upper(char* str, int size){
    for(int i = 0;i<size;i++){
        if(str[i]>='a' && str[i]<='z'){
            str[i] = str[i]-32;
        }
    }
}

void simd_upper(char* str, int size){
    __m256i bit_flip = _mm256_set1_epi8(0x20);
    __m256i a1 = _mm256_set1_epi8('a' - 1);
    __m256i b1 = _mm256_set1_epi8('z'+1);
    int i; 
    for(i = 0;i<size-32;i+=32){
        __m256i temp  = _mm256_loadu_si256((__m256i*)&str[i]);
        __m256i a = _mm256_cmpgt_epi8(temp, a1);
        __m256i b = _mm256_cmpgt_epi8(b1, temp);
        __m256i var = _mm256_and_si256(a,b);
        __m256i upper_data = _mm256_xor_si256(temp, _mm256_and_si256(bit_flip,var));
        _mm256_storeu_si256((__m256i*)&str[i], upper_data);
    }

    for(;i<size;i++){
        if(str[i]>='a' && str[i]<='z'){
            str[i] = str[i]-32;
        }
    }
}

int main(){

    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789,.-#'?!";
    size_t charset_size = sizeof(charset) - 1;
    char *big_string = (char *)malloc(s * sizeof(char));
    for (size_t i = 0; i < s - 1; i++) {
        big_string[i] = charset[rand() % charset_size];  
    }
    big_string[s - 1] = '\0';  
    clock_t start, end;

    start = clock();
    iter_upper(big_string,s);
    end = clock();
    double time_simd = ((double)(end - start));

    printf("Iter Execution Time: %f sec\n", time_simd);

    start = clock();
    iter_upper(big_string,s);
    end = clock();
    time_simd = ((double)(end - start));


    printf("SIMD Execution Time: %f sec\n", time_simd);


}