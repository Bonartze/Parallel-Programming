#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

pid_t get_pid(char *path) {
    pid_t pid = -1;
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "pidof -s %s", path);
    FILE *fp = popen(cmd, "r");
    if (fp != NULL) {
        fscanf(fp, "%d", &pid);
        pclose(fp);
    }
    return pid;
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            pid_t proc_pid = get_pid(argv[i]);
            if (kill(proc_pid, 0) != 0) {
                execv(argv[i], NULL);
                fprintf(stderr, "Error of executing of program\n");
                return -1;
            } else
                continue;
        } else if (pid > 0)
            continue;
        else {
            fprintf(stderr, "Cannot fork\n");
            return -1;
        }
    }
    return 0;
}
