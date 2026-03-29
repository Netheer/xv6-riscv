#ifndef PROCINFO_H
#define PROCINFO_H

#define PROCINFO_NAMELEN 16

enum procinfo_state {
    PS_UNUSED = 0,
    PS_USED,
    PS_SLEEPING,
    PS_RUNNABLE,
    PS_RUNNING,
    PS_ZOMBIE
};

struct procinfo {
    int pid;
    char name[PROCINFO_NAMELEN];
    int state;
    int parent_pid;
};

#endif