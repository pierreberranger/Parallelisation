#include "dist.h"

#include <math.h>
#include <stdio.h>
#include <immintrin.h> 
#include <time.h>
#include <pthread.h>



double dist(float *U, float *V, int n){
    double s = 0.0;
    for (int i = 0; i < n; i++) {
        s += sqrt(
            (U[i]*U[i] + V[i]*V[i])/(1 + U[i]*V[i]*U[i]*V[i])
        );
    }
    return s;
}

double vec_dist(float *U, float *V, int n) {
    __m256 sum = _mm256_setzero_ps();  // Registre AVX pour accumuler la somme

    int i;
    for (i = 0; i < n; i += 8) {  // Traitement par blocs de 8
        __m256 u = _mm256_load_ps(&U[i]);  // Charger 8 valeurs de U
        __m256 v = _mm256_load_ps(&V[i]);  // Charger 8 valeurs de V

        __m256 u2 = _mm256_mul_ps(u, u);  // U^2
        __m256 v2 = _mm256_mul_ps(v, v);  // V^2
        __m256 num = _mm256_add_ps(u2, v2);  // U^2 + V^2

        __m256 den = _mm256_fmadd_ps(u2, v2, _mm256_set1_ps(1.0f));  // 1 + (U * V)^2

        __m256 div = _mm256_div_ps(num, den);  // Division élément par élément

        __m256 sqrt_val = _mm256_sqrt_ps(div);  // Racine carrée

        sum = _mm256_add_ps(sum, sqrt_val);  // Accumulation des résultats
    }

    // Stocker le résultat du vecteur sum dans un tableau temporaire
    float sum_arr[8] __attribute__((aligned(32))); //Clang or gcc, pb return double precision but cannot handle it here...
    _mm256_store_ps(sum_arr, sum);
    
    // // Réduction scalaire
    double s = 0.0;
    for (int j = 0; j < 8; j++) {
        s += sum_arr[j];
    }

    return s;
}
double vec_dist_gen(float *U, float *V, int n) {
    double s = 0.0;
    __m256 sum = _mm256_setzero_ps();  // Registre AVX pour accumuler la somme

    int i;
    for (i = 0; i < n ; i += 8) {  // Traitement par blocs de 8
        __m256 u = _mm256_loadu_ps(&U[i]);  // Charger 8 valeurs de U
        __m256 v = _mm256_loadu_ps(&V[i]);  // Charger 8 valeurs de V

        __m256 u2 = _mm256_mul_ps(u, u);  // U^2
        __m256 v2 = _mm256_mul_ps(v, v);  // V^2
        __m256 num = _mm256_add_ps(u2, v2);  // U^2 + V^2

        __m256 den = _mm256_fmadd_ps(u2, v2, _mm256_set1_ps(1.0f));  // 1 + (U * V)^2

        __m256 div = _mm256_div_ps(num, den);  // Division élément par élément

        __m256 sqrt_val = _mm256_sqrt_ps(div);  // Racine carrée

        sum = _mm256_add_ps(sum, sqrt_val);  // Accumulation des résultats
    }

    // Stocker le résultat du vecteur sum dans un tableau temporaire
    float sum_arr[8];
    _mm256_storeu_ps(sum_arr, sum);
    
    // Réduction scalaire
    for (int j = 0; j < 8; j++) {
        s += sum_arr[j];
    }

    return s;
}

typedef struct {
    float *U;
    float *V;
    int chunk_size;
    double *result;
    int mode;
} ThreadData;
void* thread_func(void *arg) {
    ThreadData *data = (ThreadData*)arg;
    // Depending on the mode, call different functions
    if (data->mode == 0) {
        *(data->result) = vec_dist_gen(data->U, data->V, data->chunk_size);
    } else {
        *(data->result) = dist(data->U, data->V, data->chunk_size);
    }
    return NULL;
}
double distPar(float *U, float *V, int n, int nb_threads, int mode){
    pthread_t threads[nb_threads];
    ThreadData thread_data[nb_threads];
    double results[nb_threads];
    int chunk_size = n / nb_threads;
    

    for (int i = 0; i < nb_threads; i++) {
        thread_data[i].U = U + i * chunk_size;
        thread_data[i].V = V + i * chunk_size;
        thread_data[i].chunk_size = (i == nb_threads - 1) ? (n - i * chunk_size) : chunk_size;
        // printf("thread %d, chunksize : %d", i, thread_data[i].chunk_size);
        thread_data[i].result = &results[i];
        thread_data[i].mode = mode;

        if (pthread_create(&threads[i], NULL, thread_func, (void*)&thread_data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            return -1;
        }
    }
    // Wait for all threads to finish
    double total_result = 0.0;
    for (int i = 0; i < nb_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread %d\n", i);
            return -1;
        }
        // printf("Thread %d, %lf", i, results[i]);
        total_result += results[i];
    }

    return total_result;
}
