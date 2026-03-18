#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main(void) {
    int n;
    n = ps_listinfo(0, 0);
    if (n < 0) {
        fprintf(2, "ps_listinfo(0,0) failed\n");
        exit(1);
    }

    printf("Total processes: %d\n", n);
    struct procinfo buf[n];
    int ret = ps_listinfo(buf, n);
    if (ret < 0) {
        fprintf(2, "ps_listinfo(buf, n) failed\n");
        exit(1);
    }
    printf("copied %d records\n", ret);
    for (int i = 0; i < ret; i++) {
        printf("pid=%d ppid=%d name=%s state=%s\n",
        buf[i].pid, buf[i].parent_pid, buf[i].name, buf[i].state);
    }
    exit(0);
}