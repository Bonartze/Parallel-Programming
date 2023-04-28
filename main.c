#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool is_symmetric(double *mtr, int n, int p, int k) {
    int first = (k * n) / p;
    int last = (n * (k + 1)) / p + 1;

    for (int i = first; i < last; i++) {
        for (int j = i; j < n; j++) {
            if (mtr[i * n + j] != mtr[j * n + i]) {
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    double *mtr = malloc(sizeof(double) * 4);
    int rank, size;
    mtr[0] = 1;
    mtr[1] = 1;
    mtr[2] = 1;
    mtr[3] = 10;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {
        bool buf = true;
        MPI_Status status;
        for (int i = 1; i < size; i++) {
            MPI_Recv(&buf, sizeof(bool), MPI_C_BOOL, i, 0, MPI_COMM_WORLD, &status);
            if (!buf) {
                fprintf(stderr, "Isn't symmetric\n");
                MPI_Finalize();
                return 0;
            }
        }
        fprintf(stderr, "Is symmetric\n");
        MPI_Finalize();
        return 0;
    } else {
        bool buf = is_symmetric(mtr, 2, size, rank - 1);
        MPI_Send(&buf, sizeof(bool), MPI_C_BOOL, 0, 0, MPI_COMM_WORLD);
    }
}
