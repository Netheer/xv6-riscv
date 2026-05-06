#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

static char buf[DMSG_BUFSIZE];

int main(int argc, char* argv[]) {
    int n;

    if (argc != 1) {
        fprintf(2, "usage: dmesg\n");
        exit(1);
    }

    n = dmesg(buf, sizeof(buf));
    if (n < 0) {
        fprintf(2, "dmesg: syscall failed\n");
        exit(1);
    }

    if (n > 0) {
        write(1, buf, n);

        if(buf[n - 1] != '\n')
            write(1, "\n", 1);
    }

    exit(0);
}