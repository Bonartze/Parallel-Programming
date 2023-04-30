#include <stdio.h>
#include <mpi/mpi.h>
#include <stdlib.h>

#define N 6

void multiplying_matrix(int *a, int *b, int *c, int n, int rank, int total_processes) {
    int first = (n * rank);
    first /= total_processes;
    for (int i = first; i < n; i++)
        for (int j = first; j < n; j++) {
            c[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
}

void fill_matrix(int *mtr, int n, int rank, int total_proc) {
    int first = (n * rank);
    first /= total_proc;
    int last = n * (rank + 1);
    last /= total_proc;
    for (int i = first; i < last; i++)
        for (int j = 0; j < n; j++)
            mtr[i * n + j] = rand() % 30;
}

void print_matrix(int *mtr, int n, int rank, int total_proc) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%i ", mtr[i * n + j]);
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {
        int matrix_c_pr[N * N];
        int matrix_a_pr[N * N];
        int matrix_b_pr[N * N];
        fill_matrix(matrix_a_pr, N, 0, 1);
        fill_matrix(matrix_b_pr, N, 0, 1);
        fill_matrix(matrix_c_pr, N, 0, 1);
        if (size != 1) {
            MPI_Bcast(matrix_a_pr, N * N, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(matrix_b_pr, N * N, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(matrix_c_pr, N * N, MPI_INT, 0, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
    } else {
        size--;
        int matrix_a[N * N];
        int matrix_b[N * N];
        int matrix_c[N * N];
        MPI_Bcast(matrix_a, N * N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(matrix_b, N * N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(matrix_c, N * N, MPI_INT, 0, MPI_COMM_WORLD);
        if (rank == 1)
            print_matrix(matrix_a, N, rank - 1, size);
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 2)
            print_matrix(matrix_b, N, rank - 1, size);
        multiplying_matrix(matrix_a, matrix_b, matrix_c, N, rank - 1, size);
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 1)
            print_matrix(matrix_c, N, rank - 1, size);
    }
    MPI_Finalize();
    return 0;
}
