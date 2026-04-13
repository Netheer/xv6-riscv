#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "proc.h"
#include "defs.h"

struct file* mutexalloc (void) {
    struct file* f;
    struct sleeplock* sl;

    if ((f = filealloc()) == 0)
        return 0;

    if ((sl = (struct sleeplock*)kalloc()) == 0) {
        fileclose(f);
        return 0;
    }

    printf("mutexalloc: kalloc sleeplock at %p\n", sl);
    initsleeplock(sl, "mutex");

    f->type = FD_MUTEX;
    f->readable = 0;
    f->writable = 0;
    f->mutex = sl;

    printf("mutexalloc: created mutex fd, sleeplock %p\n", sl);
    return f;
}

void mutexclose (struct sleeplock* sl) {
    if (sl == 0)
        panic("mutexclose");
    printf("mutexclose: kfree sleeplock at %p\n", sl);
    kfree((char*)sl);
}
