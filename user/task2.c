#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void print_one(int use_mutex, int mtx, int argi, char c) {
    if (use_mutex)
        mutex_lock(mtx);

    printf("pid %d: arg %d, char %c\n", getpid(), argi, c);

    if (use_mutex)
        mutex_unlock(mtx);
}

int main(int argc, char* argv[]) {
    int use_mutex = 0;
    int mtx = -1;

    int start = 1;
    if (argc > 1 && strcmp(argv[1], "use") == 0) {
        use_mutex = 1;
        mtx = mutex();
        if (mtx < 0) {
            fprintf(2, "failed to create mutex\n");
            exit(1);
        }
        start = 2;
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    for (int i = start; i < argc; i++) {
        for (int j = 0; argv[i][j] != 0; j++) {
            print_one(use_mutex, mtx, i, argv[i][j]);
        }
    }

    if (pid > 0) {
        wait(0);
    }

    if (use_mutex)
        close(mtx);

    exit(0);
}
