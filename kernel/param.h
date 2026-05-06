#define NPROC        64  // maximum number of processes
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGBLOCKS    (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       2000  // size of file system in blocks
#define MAXPATH      128   // maximum file path name
#define USERSTACK    1     // user stack pages

#define DMSG_NPAGE 1
#define DMSG_PGSIZE 4096
#define DMSG_BUFSIZE (DMSG_NPAGE * DMSG_PGSIZE)

#define LOG_SYSCALL 0x1
#define LOG_INTR 0x2
#define LOG_PROC 0x4
#define LOG_EXEC 0x8
#define LOG_ALL 0xf
