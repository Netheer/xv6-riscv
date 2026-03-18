#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main(void) {
    struct procinfo *bad;
    bad = (struct procinfo *)1;
    int r = ps_listinfo(bad, 10);
    if (r < 0) {
        fprintf(2, "ps_listinfo(bad, 10) failed: %d\n", r);
        exit(1);
    }

    printf("badaddr_test: copied %d records\n", r);

    exit(0);
}