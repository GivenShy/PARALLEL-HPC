#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Threshold below which Fibonacci is computed sequentially
#define THRESHOLD 10

int fib(int n) {
    if (n <= 1) {
        return n;
    }

    int x, y;

    // For small n, compute sequentially
    if (n <= THRESHOLD) {
        x = fib(n - 1);
        y = fib(n - 2);
    } else {
        #pragma omp task shared(x)
        x = fib(n - 1);

        #pragma omp task shared(y)
        y = fib(n - 2);

        #pragma omp taskwait
    }

    return x + y;
}

int main() {

    int num = 20;
    int result;

    double start_time = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        {
            result = fib(num);
        }
    }

    double end_time = omp_get_wtime();

    printf("Fibonacci number F(%d) = %d\n", num, result);
    printf("Computed in %f seconds\n", end_time - start_time);

    return 0;
}
