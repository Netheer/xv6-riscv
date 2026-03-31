#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

#define MINOR_NULL 0
#define MINOR_ZERO 1
#define MINOR_URANDOM 2
#define MINOR_NULLSTAT 3

#define LCG_A 6364136223846793005ULL
#define LCG_C 1442695040888963407ULL

static uint64 urandom_seed = 12345ULL;
static struct spinlock urandom_lock;

static uint64 lcg_next(void) {
  urandom_seed = LCG_A * urandom_seed + LCG_C;
  return urandom_seed;
}

static uint64 nullstat_count = 0;
static struct spinlock nullstat_lock;

static int pseudoread(int user_dst, uint64 dst, int n, int minor) {
  if(minor == MINOR_NULL) {
    return 0;

  } else if(minor == MINOR_ZERO) {
    char buf[64];
    int i;
    for(i = 0; i < 64; i++) buf[i] = 0;
    int total = 0;
    while(total < n){
      int batch = n - total;
      if(batch > 64) batch = 64;
      if(either_copyout(user_dst, dst + total, buf, batch) < 0)
        return -1;
      total += batch;
    }
    return total;

  } else if(minor == MINOR_URANDOM) {
    int total = 0;
    while(total < n) {
      acquire(&urandom_lock);
      uint64 val = lcg_next();
      release(&urandom_lock);

      char buf[8];
      int i;
      for(i = 0; i < 8; i++)
        buf[i] = (val >> (i * 8)) & 0xFF;

      int batch = n - total;
      if(batch > 8) batch = 8;
      if(either_copyout(user_dst, dst + total, buf, batch) < 0)
        return -1;
      total += batch;
    }
    return total;

  } else if(minor == MINOR_NULLSTAT) {
    if(n != sizeof(uint64))
      return -1;
    acquire(&nullstat_lock);
    uint64 count = nullstat_count;
    release(&nullstat_lock);
    if(either_copyout(user_dst, dst, (char *)&count, sizeof(uint64)) < 0)
      return -1;
    return sizeof(uint64);
  }

  return -1;
}

static int pseudowrite(int user_src, uint64 src, int n, int minor) {
  if (minor == MINOR_NULL) {
    return n;

  } else if(minor == MINOR_ZERO) {
    return -1;

  } else if(minor == MINOR_URANDOM) {
    if (n != (int)sizeof(uint64))
      return -1;
    uint64 newseed;
    if (either_copyin((char *)&newseed, user_src, src, sizeof(uint64)) < 0)
      return -1;
    acquire(&urandom_lock);
    urandom_seed = newseed;
    release(&urandom_lock);
    return n;

  } else if(minor == MINOR_NULLSTAT) {
    acquire(&nullstat_lock);
    nullstat_count += n;
    release(&nullstat_lock);
    return n;
  }

  return -1;
}

void pseudodevinit(void) {
  initlock(&urandom_lock, "urandom");
  initlock(&nullstat_lock, "nullstat");
  devsw[PSEUDODEV].read  = pseudoread;
  devsw[PSEUDODEV].write = pseudowrite;
}
