#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct Arguments {
    double *matrix;
    int n;
    int k;
    int p;
    double *h_arr;
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

void *turn_into_matrix_thread(void *a) {
    arguments *args = a;
    static int num = 1;
    pthread_mutex_lock(&mutex);
    int first_row = (args->n * args->k);
    first_row /= args->p;
    int last_row = args->n * (args->k + 1);
    last_row = last_row / args->p;
    int ind = 0;
    for (int i = first_row; i < last_row; i++) {
        for (int j = 2; j < args->n; j++) {
            if (i + 2 < args->n && j + 2 < args->n && i != 0 && i != 1) {
                args->h_arr[ind++] = args->matrix[i * args->n + j - 2] +
                                     args->matrix[i * args->n + j + 2] +
                                     args->matrix[(i + 2) * args->n + j] + args->matrix[(i - 2) * args->n + j] -
                                     4 * args->matrix[i * args->n + j];
            }
        }
    }
    num++;
    if (num >= args->p)
        pthread_cond_broadcast(&cond);
    else {
        while (num < args->p)
            pthread_cond_wait(&cond, &mutex);
    }
    int ind1 = 0;
    for (int i = first_row; i < last_row; i++) {
        for (int j = 2; j < args->n - 2; j++) {
            if (i != 0 && i != 1) {
                args->matrix[i * args->n + j] = args->h_arr[ind1++];
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void turn_into_matrix(double *mtrx, int n, int p) {
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
        if (!(args[i].h_arr = malloc(sizeof(double) * 2 * n))) {
            fprintf(stderr, "Not enough memory\n");
            exit(2);
        }

    }
    for (int i = 0; i < p; i++)
        pthread_create(&threads[i], 0, turn_into_matrix_thread, &args[i]);
    for (int i = 0; i < p; i++)
        pthread_join(threads[i], NULL);
}

int main() {
    int n = 5, p = 5;
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
    turn_into_matrix(mtrx, n, p);
    print_matrix(mtrx, n);
    printf("\n");
    return 0;
}
