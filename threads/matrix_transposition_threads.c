#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>


void swap(double *a, double *b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

pthread_mutex_t mutex;
typedef struct Arguments {
    double *matrix;
    size_t n;
    size_t p;
    size_t k;
} args;

void print_matrix(double *matrix, size_t n) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++)
            printf("%.1f ", matrix[i * n + j]);
        printf("\n");
    }
}

void fill_matrix(double *matrix, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            matrix[i * n + j] = rand() % 100;
}

void *transposition_matrix(void *arg) {
    args *a = arg;
    pthread_mutex_lock(&mutex);
    size_t first_row = (a->n * a->k);
    first_row /= a->p;
    size_t last_row = a->n * (a->k + 1);
    last_row = last_row / a->p;
    //  last_row = min(last_row, a->n);
    for (size_t i = first_row; i < last_row; i++) {
        for (size_t j = i; j < a->n; j++) {
            swap(&a->matrix[i * a->n + j], &a->matrix[j * a->n + i]);
        }
    }
    pthread_mutex_unlock(&mutex);
}

int main() {
    int n, p;
    printf("Enter number of threats and matrix size: ");
    scanf("%i %i", &p, &n);
    double *mtrx;
    if (!(mtrx = malloc(sizeof(double) * n * n))) {
        fprintf(stderr, "Not enough memory\n");
        return 2;
    }
    args *arg;
    if (!(arg = malloc(sizeof(args) * p))) {
        fprintf(stderr, "Not enough memory\n");
    }
    for (size_t i = 0; i < p; i++) {
        arg[i].n = n;
        arg[i].p = p;
        arg[i].matrix = mtrx;
        arg[i].k = i;
    }
    fill_matrix(mtrx, n);
    print_matrix(mtrx, n);
    printf("\n");
    if (pthread_mutex_init(&mutex, 0)) {
        fprintf(stderr, "Cannot init mutex\n");
        return 1;
    }
    pthread_t *threads;
    if (!(threads = malloc(sizeof(pthread_t) * p))) {
        fprintf(stderr, "Not enough memory\n");
    }
    for (size_t i = 0; i < p; i++)
        if (pthread_create(&threads[i], NULL, transposition_matrix, &arg[i])) {
            fprintf(stderr, "Cannot create threat\n");
            return 1;
        }
    for (size_t i = 0; i < p; i++)
        pthread_join(threads[i], NULL);
    print_matrix(mtrx, n);
    return 0;
}
