#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
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


void *turn_into_sum_thread(void *args) {
    pthread_mutex_lock(&mutex);
    arguments *args_ = args;
    int first_row = (args_->n * args_->k);
    first_row /= args_->p;
    int last_row = args_->n * (args_->k + 1);
    last_row = last_row / args_->p;
    for (int i = first_row; i < last_row; i++) {
        for (int j = i; j < args_->n; j++) {
            args_->matrix[i * args_->n + j] = (args_->matrix[i * args_->n + j] + args_->matrix[j * args_->n + i]) / 2;
            args_->matrix[j * args_->n + i] = args_->matrix[i * args_->n + j];
        }
    }
    pthread_mutex_unlock(&mutex);
}

void turn_into_sum(double *mtrx, int n, int p) {
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
        pthread_create(&threads[i], 0, turn_into_sum_thread, &args[i]);
    for (int i = 0; i < p; i++)
        pthread_join(threads[i], NULL);
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
    fill_matrix(mtrx, n);
    print_matrix(mtrx, n);
    printf("\n");
    turn_into_sum(mtrx, n, p);
    print_matrix(mtrx, n);
    printf("\n");
    return 0;
}
