#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void print_unsync(int argi, char *word) {
    int pid = getpid();
    for (int j = 0; word[j]; j++) {
        printf("pid %d: arg %d, char %c\n", pid, argi, word[j]);
    }
}

void print_sync(int mtx, int argi, char *word) {
    int pid = getpid();
    for (int j = 0; word[j]; j++) {
        mutex_lock(mtx);
        printf("pid %d: arg %d, char %c\n", pid, argi, word[j]);
        mutex_unlock(mtx);
    }
}

void run_test(int argc, char **argv, int use_mutex) {
    int mtx = -1;
    if (use_mutex) {
        mtx = mutex();
        if (mtx < 0) {
            fprintf(2, "mutex() failed\n");
            exit(1);
        }
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        if (use_mutex)
            print_sync(mtx, i, argv[i]);
        else
            print_unsync(i, argv[i]);
    }

    if (pid > 0)
        wait(0);

    if (use_mutex)
        close(mtx);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: task2 word1 word2 ...\n");
        exit(1);
    }

    printf("=== Without mutex ===\n");
    run_test(argc, argv, 0);

    printf("=== With mutex ===\n");
    run_test(argc, argv, 1);

    exit(0);
}
