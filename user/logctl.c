#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

static int streq(char* a, char* b) {
    return strcmp(a, b) == 0;
}

static int atoi_nonneg(char *s) {
    int n;

    n = atoi(s);
    if (n < 0)
        return -1;

    return n;
}

static void usage() {
    fprintf(2, "usage: logctl off\n");
    fprintf(2, "usage: logctl all [ticks]\n");
    fprintf(2, "usage: logctl syscall|intr|proc|exec ... [ticks]\n");
    exit(1);
}

int main(int argc, char* argv[]) {
    int mask;
    int nticks;
    int i;

    if (argc < 2)
        usage();

    if (streq(argv[1], "off")) {
        if (argc != 2)
            usage();

        if (logctl(0, 0) < 0) {
            fprintf(2, "logctl: syscall failed\n");
            exit(1);
        }

        exit(0);
    }

    mask = 0;
    nticks = 0;

    for (i = 1; i < argc; i++) {
        if (streq(argv[i], "all")) {
            mask |= LOG_ALL;
        } else if (streq(argv[i], "syscall")) {
            mask |= LOG_SYSCALL;
        } else if (streq(argv[i], "intr")) {
            mask |= LOG_INTR;
        } else if (streq(argv[i], "proc")) {
            mask |= LOG_PROC;
        } else if (streq(argv[i], "exec")) {
            mask |= LOG_EXEC;
        } else {
            nticks = atoi_nonneg(argv[i]);
            if (nticks < 0)
                usage();

            if (i != argc - 1)
                usage();
        }
    }

    if (mask == 0)
        usage();

    if (logctl(mask, nticks) < 0) {
        fprintf(2, "logctl: syscall failed\n");
        exit(1);
    }

    exit(0);
}