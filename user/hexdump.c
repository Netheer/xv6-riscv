#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static void put_hex_byte(int b) {
  const char *hex = "0123456789ABCDEF";
  char buf[2];
  buf[0] = hex[(b >> 4) & 0xF];
  buf[1] = hex[b & 0xF];
  write(1, buf, 2);
}

int main(int argc, char *argv[]) {
  if(argc != 3) {
    fprintf(2, "Usage: hexdump <n> <file>\n");
    exit(1);
  }

  int n = atoi(argv[1]);
  if(n <= 0) {
    fprintf(2, "hexdump: n must be > 0\n");
    exit(1);
  }

  int fd = open(argv[2], O_RDONLY);
  if(fd < 0) {
    fprintf(2, "hexdump: cannot open %s\n", argv[2]);
    exit(1);
  }

  int total = 0;
  while(total < n) {
    char buf[64];
    int want = n - total;
    if(want > 64) want = 64;
    int got = read(fd, buf, want);
    if(got < 0) {
      fprintf(2, "hexdump: read error\n");
      close(fd);
      exit(1);
    }
    if(got == 0) break;
    for(int i = 0; i < got; i++){
      if(total + i > 0)
        write(1, " ", 1);
      put_hex_byte((unsigned char)buf[i]);
    }
    total += got;
  }
  write(1, "\n", 1);

  close(fd);
  exit(0);
}
