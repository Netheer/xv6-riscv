#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main(void) {
    int n;
    n = ps_listinfo(0, 0);

    if (n < 0) {
        fprintf(2, "ps_listinfo(0, 0) failed: %d\n", n);
        exit(1);
    }

    if (n == 0) {
        exit(0);
    }

    struct procinfo buf[n];
    int r = ps_listinfo(buf, n);

    if (r < 0) {
        fprintf(2, "ps_listinfo(buf, %d) failed: %d\n", n, r);
        exit(1);
    }

    printf("pid\tname\tstate    ppid    pname\n");

    for (int i = 0; i < r; i++) {
        printf("%d\t%s\t%s   %d      %s\n",
        buf[i].pid, buf[i].name, buf[i].state, buf[i].parent_pid, buf[i].pname);
    }

    exit(0);
}