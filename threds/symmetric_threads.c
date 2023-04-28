#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct ARGUMENT {
    int *matrix;
    size_t n;
    size_t p;
    size_t k;
} argS;

void print_matrix(int *matrix, size_t n) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++)
            printf("%i ", matrix[i * n + j]);
        printf("\n");
    }
}

void fill_matrix(int *matrix, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            matrix[i * n + j] = rand() % 100;
}

void *is_symmetric_thread(void *args) {
    argS *args_ = args;
    if (pthread_mutex_lock(&mutex) != 0) {
        fprintf(stderr, "Cannot lock mutex\n");
        exit(1);
    }
    size_t first_row = (args_->n * args_->k);
    first_row /= args_->p;
    size_t last_row = args_->n * (args_->k + 1);
    last_row = last_row / args_->p;
    for (size_t i = first_row; i < last_row; i++) {
        for (size_t j = i; j < args_->n; j++) {
            if (args_->matrix[i * args_->n + j] != args_->matrix[j * args_->n + i]) {
                if (pthread_mutex_unlock(&mutex) != 0) {
                    fprintf(stderr, "Cannot unlock mutex\n");
                    exit(1);
                }
                pthread_exit(false);
            }
        }
    }
    if (pthread_mutex_unlock(&mutex) != 0) {
        fprintf(stderr, "Cannot unlock mutex\n");
        exit(1);
    }
    pthread_exit((void *) true);
}

bool is_symmetric(argS *args) {
    pthread_t *threads;
    if (!(threads = malloc(sizeof(pthread_t) * args[0].p))) {
        fprintf(stderr, "Not enough memory\n");
        exit(1);
    }
    for (size_t i = 0; i < args[0].p; i++)
        if (pthread_create(&threads[i], 0, is_symmetric_thread, &args[i]) != 0) {
            fprintf(stderr, "Cannot create a thread\n");
            exit(1);
        }
    for (size_t i = 0; i < args[0].p; i++) {
        bool ret;
        if (pthread_join(threads[i], &ret) != 0) {
            fprintf(stderr, "Cannot wait a thread\n");
            exit(2);
        }
        if (!((bool) ret))
            return false;
    }
    return true;
}

int main() {
    int n, p;
    printf("Enter number of threats and matrix size: ");
    scanf("%i %i", &p, &n);
    int *mtrx;
    if (!(mtrx = malloc(sizeof(int) * n * n))) {
        fprintf(stderr, "Not enough memory\n");
        return 2;
    }
    argS *arg;
    if (!(arg = malloc(sizeof(argS) * p))) {
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
    printf("%s", is_symmetric(arg) ? "symmetric" : "not symmetric");
    printf("\n");
    return 0;
}
