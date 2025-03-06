// gcc -mavx2 -mfma main.c dist.c -lm -lpthread -o main; ./main <nb_threads> <mode>
#include "dist.h"
#include <math.h>
#include <stdio.h>
#include <immintrin.h> 
#include <time.h>
#include <pthread.h>

#define ALIGNMENT 32 

float rand_float_in_range(float a, float b) {
    return a + ((float)rand() / RAND_MAX) * (b - a);  
}

double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1.0e6;  // Convert to milliseconds
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <nb_threads> <mode>\n", argv[0]);
        return 1;
    }

    int n = (int)pow(1024, 2);
    float *U, *V;
    posix_memalign((void**)&U, ALIGNMENT, n * sizeof(float));
    posix_memalign((void**)&V, ALIGNMENT, n * sizeof(float));

    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        U[i] = rand_float_in_range(0.0f, 1.0f);
        V[i] = rand_float_in_range(0.0f, 1.0f);
    }

    struct timespec start, end;
    double result, vec_result, vec_gen_result, par_result, exec_time;


   

    // Measure time for vec_dist_gen(U, V, n)
    clock_gettime(CLOCK_MONOTONIC, &start);
    vec_gen_result = vec_dist_gen(U, V, n);
    clock_gettime(CLOCK_MONOTONIC, &end);
    exec_time = get_time_diff(start, end);
    printf("[Vectorized Unaligned] vec_dist_gen(U, V, n) = %lf, Execution time: %.2f ms\n", vec_gen_result, exec_time);

    // Measure time for dist(U, V, n)
    clock_gettime(CLOCK_MONOTONIC, &start);
    result = dist(U, V, n);
    clock_gettime(CLOCK_MONOTONIC, &end);
    exec_time = get_time_diff(start, end);
    printf("[Sequential] dist(U, V, n) = %lf, Execution time: %.2f ms\n", result, exec_time);

     // Measure time for vec_dist(U, V, n)
     clock_gettime(CLOCK_MONOTONIC, &start);
     vec_result = vec_dist(U, V, n);
     clock_gettime(CLOCK_MONOTONIC, &end);
     exec_time = get_time_diff(start, end);
     printf("[Vectorized] vec_dist(U, V, n) = %lf, Execution time: %.2f ms\n", vec_result, exec_time);


    // Measure time for distPar(U, V, n)
    int nb_threads = atoi(argv[1]);
    int mode = atoi(argv[2]);

    clock_gettime(CLOCK_MONOTONIC, &start);
    par_result = distPar(U, V, n, nb_threads, mode);
    clock_gettime(CLOCK_MONOTONIC, &end);
    exec_time = get_time_diff(start, end);
    printf("[Parallel] distPar(U, V, n) with %d threads, mode %d = %lf, Execution time: %.2f ms\n", 
           nb_threads, mode, par_result, exec_time);

    return 0;
}
