#ifndef PROCINFO_H
#define PROCINFO_H

#define PNAME_LEN 16

#define PSTATE_LEN 16

struct procinfo {
    int pid;
    char name[PNAME_LEN];
    char pname[PNAME_LEN];
    char state[PSTATE_LEN];
    int parent_pid;
};

#endif