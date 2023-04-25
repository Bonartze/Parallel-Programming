#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_out = PTHREAD_COND_INITIALIZER;
pthread_cond_t mutex_out = PTHREAD_COND_INITIALIZER;
typedef struct Arguments {
    double *matrix;
    int n;
    int k;
    int p;
} arguments;

void print_matrix(double *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%.1f ", matrix[i * n + j]);
        printf("\n");
    }
}

void fill_matrix(double *matrix, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            matrix[i * n + j] = rand() % 100;
}

void *average_arifmetic_thread(void *arg) {
    arguments *args = arg;
    static int num = 0;
    static int num_out = 1;
    pthread_mutex_lock(&mutex);
    num++;
    if (num >= args->p) {
        pthread_cond_broadcast(&cond);
    } else {
        while (num < args->p) {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    int first_row = (args->n * args->k);
    first_row /= args->p;
    int last_row = args->n * (args->k + 1);
    last_row = last_row / args->p;
    for (size_t i = first_row; i < last_row; i++) {
        double temp = 0.0;
        for (size_t j = 1; j < args->n - 1; j++) {
            if (j == 1) {
                temp = args->matrix[i * args->n + j];
                args->matrix[i * args->n + j] =
                        (args->matrix[i * args->n + j - 1] + args->matrix[i * args->n + j + 1]) / 2;
            } else {
                double t = temp;
                temp = args->matrix[i * args->n + j];
                args->matrix[i * args->n + j] = (t + args->matrix[i * args->n + j + 1]) / 2;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void average_arifmetic(double *mtrx, int n, int p) {
    arguments *args;
    if (!(args = malloc(sizeof(arguments) * p))) {
        fprintf(stderr, "Not enough memory\n");
        exit(2);
    }
    pthread_t *threads;
    if (!(threads = malloc(sizeof(pthread_t) * p))) {
        fprintf(stderr, "Not enough memory\n");
        exit(2);
    }
    for (int i = 0; i < p; i++) {
        args[i].matrix = mtrx;
        args[i].n = n;
        args[i].p = p;
        args[i].k = i;
    }
    for (int i = 0; i < p; i++)
        pthread_create(&threads[i], 0, average_arifmetic_thread, &args[i]);
    for (int i = 0; i < p; i++)
        pthread_join(threads[i], NULL);
}

int main() {
    int n = 4, p = 3;
    printf("Enter number of threats and matrix size: ");
    scanf("%i %i", &p, &n);
    double *mtrx;
    if (!(mtrx = malloc(sizeof(double) * n * n))) {
        fprintf(stderr, "Not enough memory\n");
        return 2;
    }
    fill_matrix(mtrx, n);
    print_matrix(mtrx, n);
    printf("\n");
    average_arifmetic(mtrx, n, p);
    print_matrix(mtrx, n);
    printf("\n");
    return 0;
}