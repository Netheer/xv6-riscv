#ifndef MUTEX_H
#define MUTEX_H

#include "spinlock.h"

struct proc;

struct mutex {
    struct spinlock lock;
    int locked;
    struct proc *owner;
};

#endif