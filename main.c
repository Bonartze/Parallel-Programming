
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <wait.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>

#define key 1221
#define sem_key 1232
static int shm_id;
static int sem_id;
struct sembuf lock_sem = {0, -1, 0};
struct sembuf unlock_sem = {0, 1, 0};

static int sem_lock() {
    if (semop(sem_id, &lock_sem, 1) == -1) {
        fprintf(stderr, "Cannot lock semaphore\n");
        return 1;
    }
    return 0;
}

static int sem_unlock() {
    if (semop(sem_id, &unlock_sem, 1) == -1) {
        fprintf(stderr, "Cannot unlock semaphore\n");
        return 1;
    }
    return 0;
}

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *_buf;
};
typedef struct shared_memory {
    int i;
    char *file_name_in;
} SHM;

void read_write_in(char *name, char *name_out) {
    FILE *out = fopen(name_out, "aw");
    FILE *in = fopen(name, "r");
    int res = 0, temp = 0;
    while (fscanf(in, "%i", &temp) == 1)
        res += temp;
    fprintf(out, "%i ", res);
    fclose(in);
    fclose(out);
}

int main(int argc, char *argv[]) {
    SHM *data;
    if ((shm_id = shmget(key, sizeof(SHM), IPC_CREAT | 0664)) < 0) {
        fprintf(stderr, "Cannot get shared memory\n");
        return 1;
    }
    if ((data = (void *) shmat(shm_id, NULL, 0)) == (void *) (-1)) {
        fprintf(stderr, "Cannot attach shared memory\n");
        return 2;
    }
    data->file_name_in = "res.txt";
    read_write_in(argv[1], data->file_name_in);
    union semun arg;
    if ((sem_id = semget(sem_key, 1, IPC_CREAT | 0666)) < 0) {
        fprintf(stderr, "Cannot get semaphore\n");
        return 3;
    }
    arg.val = 1;
    if (semctl(sem_id, 0, SETVAL, arg) < 0) {
        fprintf(stderr, "Cannot set value\n");
        return -1;
    }
    data->i = 2;
    if (shmdt(data) == -1) {
        fprintf(stderr, "Cannot detach memory\n");
        return -1;
    }
    pid_t pid;
    for (int j = 0; j < argc - 2; j++) {
        pid = fork();
        if (pid == 0) {
            SHM *memory;
            if (sem_lock())
                return -1;
            if ((memory = (void *) shmat(shm_id, 0, 0)) == (void *) -1) {
                fprintf(stderr, "Cannot attach shared memory2\n");
                return 1;
            }
            read_write_in(argv[memory->i++], memory->file_name_in);
            if (shmdt(memory) == -1) {
                fprintf(stderr, "Cannot detach shared memory\n");
                return -1;
            }
            if (sem_unlock())
                return -1;
            return 0;
        }
    }
    return 0;
}