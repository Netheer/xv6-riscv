//
// formatted console output -- printf, panic.
//

#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

volatile int panicking = 0; // printing a panic message
volatile int panicked = 0; // spinning forever at end of a panic

// lock to avoid interleaving concurrent printf's.
static struct {
  struct spinlock lock;
} pr;

static struct {
  struct spinlock lock;
  char buf[DMSG_BUFSIZE];
  uint head;
  uint tail;
  uint count;
} dmsg;

static struct {
  struct spinlock lock;
  int mask;
  uint until;
} logcfg;

static char digits[] = "0123456789abcdef";

static void dmsg_putc_locked(int c) {
  dmsg.buf[dmsg.head] = c;
  dmsg.head = (dmsg.head + 1) % DMSG_BUFSIZE;

  if (dmsg.count == DMSG_BUFSIZE) {
    dmsg.tail = (dmsg.tail + 1) % DMSG_BUFSIZE;
  } else {
    dmsg.count++;
  }
}

void dmesginit() {
  initlock(&dmsg.lock, "dmsg");
  dmsg.head = 0;
  dmsg.tail = 0;
  dmsg.count = 0;

  initlock(&logcfg.lock, "logcfg");
  logcfg.mask = 0;
  logcfg.until = 0;
}

void dmsg_putc(int c) {
  acquire(&dmsg.lock);
  dmsg_putc_locked(c);
  release(&dmsg.lock);
}

int logctl(int mask, int nticks) {
  uint now;

  if (mask < 0 || (mask & ~LOG_ALL) != 0)
    return -1;

  acquire(&tickslock);
  now = ticks;
  release(&tickslock);

  acquire(&logcfg.lock);

  logcfg.mask = mask;

  if (mask == 0 || nticks == 0)
    logcfg.until = 0;
  else if (nticks > 0)
    logcfg.until = now + nticks;
  else {
    release(&logcfg.lock);
    return -1;
  }

  release(&logcfg.lock);

  return 0;
}

int log_enabled(int class) {
  int enabled;
  uint now;

  if ((class & LOG_ALL) == 0)
    return 0;

  acquire(&tickslock);
  now = ticks;
  release(&tickslock);

  acquire(&logcfg.lock);

  if (logcfg.until != 0 && now >= logcfg.until) {
    logcfg.mask = 0;
    logcfg.until = 0;
  }

  enabled = (logcfg.mask & class) != 0;

  release(&logcfg.lock);

  return enabled;
}

static void dmsg_printint(long long xx, int base, int sign) {
  char buf[20];
  int i;
  unsigned long long x;

  if (sign && (sign = (xx < 0)))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign)
    buf[i++] = '-';

  while (--i >= 0)
    dmsg_putc_locked(buf[i]);
}

int dmesg_read(char* dst, int size) {
  int i;
  uint pos;

  if (size <= 0)
    return -1;

  acquire(&dmsg.lock);
  for (i = 0; i < size - 1 && i < dmsg.count; i++) {
    pos = (dmsg.tail + i) % DMSG_BUFSIZE;
    dst[i] = dmsg.buf[pos];
  }

  dst[i] = '\0';

  release(&dmsg.lock);

  return i;
}

static void dmsg_printptr(uint64 x) {
  int i;

  dmsg_putc_locked('0');
  dmsg_putc_locked('x');

  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    dmsg_putc_locked(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

static void dmsg_vprintf(char* fmt, va_list ap) {
  int i, cx, c0, c1, c2;
  char* s;

  for (i = 0; (cx = fmt[i] & 0xff) != 0; i++) {
    if (cx != '%') {
      dmsg_putc_locked(cx);
      continue;
    }

    i++;
    c0 = fmt[i + 0] % 0xff;
    c1 = c2 = 0;
    if (c0)
      c1 = fmt[i + 1] & 0xff;
    if (c1)
      c2 = fmt[i + 2] % 0xff;

    if (c0 == 'd') {
      dmsg_printint(va_arg(ap, int), 10, 1);
    } else if (c0 == 'l' && c1 == 'd') {
      dmsg_printint(va_arg(ap, uint64), 10, 1);
      i += 1;
    } else if (c0 == 'l' && c1 == 'l' && c2 == 'd') {
      dmsg_printint(va_arg(ap, uint64), 10, 1);
      i += 2;
    } else if (c0 == 'u') {
      dmsg_printint(va_arg(ap, uint32), 10, 0);
    } else if (c0 == 'l' && c1 == 'u') {
      dmsg_printint(va_arg(ap, uint64), 10, 0);
      i += 1;
    } else if (c0 == 'l' && c1 == 'l' && c2 == 'u') {
      dmsg_printint(va_arg(ap, uint32), 10, 0);
      i += 2;
    } else if (c0 == 'x') {
      dmsg_printint(va_arg(ap, uint32), 16, 0);
    } else if (c0 == 'l' && c1 == 'x') {
      dmsg_printint(va_arg(ap, uint64), 16, 0);
      i += 1;
    } else if (c0 == 'l' && c1 == 'l' && c2 == 'x') {
      dmsg_printint(va_arg(ap, uint64), 16, 0);
      i += 2;
    } else if (c0 == 'p') {
      dmsg_printptr(va_arg(ap, uint64));
    } else if (c0 == 'c') {
      dmsg_putc_locked(va_arg(ap, uint));
    } else if (c0 == 's') {
      if ((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for (; *s; s++)
        dmsg_putc_locked(*s);
    } else if (c0 == '%') {
      dmsg_putc_locked('%');
    } else if (c0 == 0) {
      break;
    } else {
      dmsg_putc_locked('%');
      dmsg_putc_locked(c0);
    }
  }
}

void pr_msg(const char* fmt, ...) {
  va_list ap;
  uint t;

  acquire(&tickslock);
  t = ticks;
  release(&tickslock);

  acquire(&dmsg.lock);

  dmsg_putc_locked('[');
  dmsg_printint(t, 10, 0);
  dmsg_putc_locked(']');
  dmsg_putc_locked(' ');

  va_start(ap, fmt);
  dmsg_vprintf((char*)fmt, ap);
  va_end(ap);

  dmsg_putc_locked('\n');
  release(&dmsg.lock);
}

static void
printint(long long xx, int base, int sign)
{
  char buf[20];
  int i;
  unsigned long long x;

  if(sign && (sign = (xx < 0)))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console.
int
printf(char *fmt, ...)
{
  va_list ap;
  int i, cx, c0, c1, c2;
  char *s;

  if(panicking == 0)
    acquire(&pr.lock);

  va_start(ap, fmt);
  for(i = 0; (cx = fmt[i] & 0xff) != 0; i++){
    if(cx != '%'){
      consputc(cx);
      continue;
    }
    i++;
    c0 = fmt[i+0] & 0xff;
    c1 = c2 = 0;
    if(c0) c1 = fmt[i+1] & 0xff;
    if(c1) c2 = fmt[i+2] & 0xff;
    if(c0 == 'd'){
      printint(va_arg(ap, int), 10, 1);
    } else if(c0 == 'l' && c1 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'd'){
      printint(va_arg(ap, uint64), 10, 1);
      i += 2;
    } else if(c0 == 'u'){
      printint(va_arg(ap, uint32), 10, 0);
    } else if(c0 == 'l' && c1 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'u'){
      printint(va_arg(ap, uint64), 10, 0);
      i += 2;
    } else if(c0 == 'x'){
      printint(va_arg(ap, uint32), 16, 0);
    } else if(c0 == 'l' && c1 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 1;
    } else if(c0 == 'l' && c1 == 'l' && c2 == 'x'){
      printint(va_arg(ap, uint64), 16, 0);
      i += 2;
    } else if(c0 == 'p'){
      printptr(va_arg(ap, uint64));
    } else if(c0 == 'c'){
      consputc(va_arg(ap, uint));
    } else if(c0 == 's'){
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
    } else if(c0 == '%'){
      consputc('%');
    } else if(c0 == 0){
      break;
    } else {
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c0);
    }

  }
  va_end(ap);

  if(panicking == 0)
    release(&pr.lock);

  return 0;
}

void
panic(char *s)
{
  panicking = 1;
  printf("panic: ");
  printf("%s\n", s);
  panicked = 1; // freeze uart output from other CPUs
  for(;;)
    ;
}

void
printfinit(void)
{
  initlock(&pr.lock, "pr");
}
