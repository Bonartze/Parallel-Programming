#include <stdio.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <wait.h>

#define KEY 1221
#define SIZE 1000
typedef struct Stack {
    int size;
    int St[SIZE];
} ST;
static int sem_id;
static int shm_id;
struct sembuf lock_sem = {0, -1, 0};
struct sembuf unlock_sem = {0, 1, 0};
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *_buf;
};
union semun arg;

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

void push(int element) {
    sem_lock();
    ST *s;
    if ((s = shmat(shm_id, NULL, 0)) == (ST *) (-1)) {
        fprintf(stderr, "Cannot attach to current process");
        exit(-1);
    }
    // fprintf(stderr, "%i\n", s->size);

    if (s->size == 0)
        memset(s->St, 0, sizeof(s->St));
    if (s->size >= SIZE - 1) {
        fprintf(stderr, "Stack is full\n");
        return;
    }
    s->St[s->size] = element;
    s->size++;
    if (shmdt(s) < 0) {
        fprintf(stderr, "Cannot detach\n");
        exit(-1);
    }
    sem_unlock();
}

int pop() {
    sem_lock();
    ST *s;
    if ((s = shmat(shm_id, NULL, 0)) == (ST *) -1) {
        fprintf(stderr, "Cannot attach to current process\n");
        exit(-1);
    }
    int temp = s->St[--s->size];
    s->St[s->size] = -1;
    if (shmdt(s) < 0) {
        fprintf(stderr, "Cannot detach\n");
        exit(-1);
    }
    sem_unlock();
    return temp;
}

int main(int argc, char *argv[]) {  //numbers from 0 to 9
    ST *stack;
    if ((shm_id = shmget(KEY, sizeof(ST), IPC_CREAT | 0666)) < 0) {
        fprintf(stderr, "Cannot get shared memory\n");
        return 1;
    }
    if ((stack = shmat(shm_id, NULL, 0)) == (ST *) -1) {
        fprintf(stderr, "Cannot attach to current process\n");
        return -1;
    }
    stack->size = 0;
    if ((sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666)) < 0) {
        fprintf(stderr, "Cannot get semaphore\n");
        return 3;
    }
    arg.val = 1;
    if (semctl(sem_id, 0, SETVAL, arg) < 0) {
        fprintf(stderr, "Cannot set value\n");
        return -1;
    }
    if (shmdt(stack) < 0) {
        fprintf(stderr, "Cannot detach\n");
        return -1;
    }
    for (int i = 1; i < argc; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            push(*argv[i] - '0');
            return 0;
        } else if (pid > 0)
            continue;
        else {
            fprintf(stderr, "Cannot fork\n");
            return -1;
        }
    }
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, 0)) > 0);
    if ((stack = shmat(shm_id, NULL, 0)) == (ST *) -1) {
        fprintf(stderr, "Cannot attach to current process\n");
        return -1;
    }
    fprintf(stderr, "Size is %i\n", stack->size);
    for (int i = stack->size - 1; i >= 0; i--) {
        fprintf(stderr, "%i: %i\n", i, pop());
    }
    if (semctl(sem_id, 0, IPC_RMID, 0) < 0) {
        fprintf(stderr, "Cannot remove\n");
        return -1;
    }
    if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
        fprintf(stderr, "Cannot remove\n");
        return -1;
    }
    return 0;
}
