#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <wait.h>

#ifndef UNTITLED40_MQ_H
#define UNTITLED40_MQ_H
struct msg {
    long mtype;
    int number;
};

#endif

static int msg_id;

void check_file(char *name) {

    struct msg buf;
    buf.mtype = 1;
    FILE *f = fopen(name, "r");
    if (!f) {
        fprintf(stderr, "Cannot open the file\n");
        exit(1);
    }
    int last = 0, cur = 0;
    fscanf(f, "%i", &last);
    while (fscanf(f, "%i", &cur) == 1 && last == cur) {
        last = cur;
        fscanf(f, "%i", &cur);
    }
    if (last != cur) {
        buf.number = -1;
        if (msgsnd(msg_id, &buf, sizeof(int), 0) < 0) {
            fprintf(stderr, "Cannot send msg in queue\n");
            fclose(f);
            exit(1);
        };
        fclose(f);
        return;
    } else {
        buf.number = 1;
        if (msgsnd(msg_id, &buf, sizeof(int), 0) < 0) {
            fprintf(stderr, "Cannot send msg in queue\n");
            fclose(f);
            exit(1);
        };
    }
    fclose(f);
}

void number_of_steady_ranges(int n, char *names[]) {
    for (int i = 1; i < n; i++) {
        pid_t pid = fork();
        if (pid > 0)
            continue;
        else if (pid < 0) {
            fprintf(stderr, "Cannot fork");
            exit(1);
        } else {
            check_file(names[i]);
            exit(0);
        }
    }
}

int main(int argc, char *argv[]) {
    struct msg m;
    msg_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    number_of_steady_ranges(argc, argv);
    int status;
    while (wait(&status) > 0);
    while (msgrcv(msg_id, &m, sizeof(m) - sizeof(long), 0, IPC_NOWAIT) > 0)
        fprintf(stderr, "%i\n", m.number);
    msgctl(msg_id, IPC_RMID, NULL);
    return 0;
}
