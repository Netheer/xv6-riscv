#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main(void) {
    struct procinfo buf[1];
    int r = ps_listinfo(buf, 1);
    if (r < 0) {
        fprintf(2, "small_test: ps_listinfo(buf, 1) failed: %d\n", r);
        exit(1);
    }

    printf("small_test: copied %d records\n", r);
    exit(0);
}