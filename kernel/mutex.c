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
#include "mutex.h"

struct file* mutexalloc(void) {
    struct file* f;
    struct mutex* m;

    if ((f = filealloc()) == 0)
        return 0;

    if ((m = (struct mutex*)kalloc()) == 0) {
        fileclose(f);
        return 0;
    }

    initlock(&m->lock, "mutex");
    m->locked = 0;
    m->owner = 0;

    f->type = FD_MUTEX;
    f->readable = 0;
    f->writable = 0;
    f->mutex = m;

    return f;
}

void mutexclose(struct mutex* m) {
    if (m == 0)
        panic("mutexclose");

    kfree((char*)m);
}