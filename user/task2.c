#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void write_all(int fd, const char* buffer, int n) {
    int off = 0;
    while (off < n) {
        int m = write(fd, buffer + off, n - off);
        if (m < 0) {
            fprintf(2, "task2: write failed\n");
            exit(1);
        }
        off += m;
    }
}

int main(int argc, char* argv[]) {
    int p[2];
    if (pipe(p) < 0) {
        fprintf(2, "task2: pipe failed\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "task2: fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        close(p[1]);
        close(0);
        if (dup(p[0]) < 0) {
            fprintf(2, "task2: dup failed\n");
            exit(1);
        }
        close(p[0]);

        char* wc_argv[] = { "wc", 0 };
        exec("wc", wc_argv);
        fprintf(2, "task2: exec wc failed\n");
        exit(1);
    }

    close(p[0]);

    for (int i = 1; i < argc; ++i) {
        int n = strlen(argv[i]);
        write_all(p[1], argv[i], n);
        write_all(p[1], "\n", 1);
    }

    close(p[1]);

    wait(0);
    exit(0);
}