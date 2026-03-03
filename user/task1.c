#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int pseudo_random(int seed) {
    seed = seed * 214013 + 2531011;
    return seed & 0x7fffffff;
}

int main (int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(2, "Usage: task1 a|b\n");
        exit(1);
    }

    int mode_b = 0;
    if (argv[1][0] == 'b') {
        mode_b = 1;
    } else if (argv[1][0] == 'a') {
    } else {
        fprintf(2, "Usage: task1 a|b\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        int seed = getpid();
        int r = pseudo_random(seed);

        int ticks = 50 + (r % 101);

        fprintf(2, "child: pid = %d sleeping %d ticks (~%d seconds)\n", getpid(), ticks, ticks / 10);
        pause(ticks);
        printf("child: pid = %d exit(1)\n", getpid());
        exit(1);
    }

    printf("parent: pid = %d child = %d\n", getpid(), pid);
    if (mode_b) {
        printf("parent: killing child %d\n", pid);
        kill(pid);
    }

    int status = 0;
    int wpid = wait(&status);

    printf("parent: wait returned pid = %d, status = %d\n", wpid, status);
    exit(0);
}