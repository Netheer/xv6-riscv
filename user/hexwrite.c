#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static int hex_digit(char c) {
  if(c >= '0' && c <= '9') return c - '0';
  if(c >= 'a' && c <= 'f') return c - 'a' + 10;
  if(c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

int main(int argc, char *argv[]) {
  if(argc != 3) {
    fprintf(2, "Usage: hexwrite <hexstring> <file>\n");
    exit(1);
  }

  char *hex = argv[1];
  int hexlen = strlen(hex);
  if(hexlen % 2 != 0) {
    fprintf(2, "hexwrite: hex string must have even length\n");
    exit(1);
  }

  int nbytes = hexlen / 2;
  char buf[256];
  if(nbytes > 256) {
    fprintf(2, "hexwrite: hex string too long (max 256 bytes)\n");
    exit(1);
  }

  for(int i = 0; i < nbytes; i++) {
    int hi = hex_digit(hex[2*i]);
    int lo = hex_digit(hex[2*i + 1]);
    if(hi < 0 || lo < 0){
      fprintf(2, "hexwrite: invalid hex character\n");
      exit(1);
    }
    buf[i] = (hi << 4) | lo;
  }

  int fd = open(argv[2], O_WRONLY);
  if(fd < 0) {
    fprintf(2, "hexwrite: cannot open %s\n", argv[2]);
    exit(1);
  }

  int r = write(fd, buf, nbytes);
  if(r != nbytes) {
    fprintf(2, "Write error\n");
    close(fd);
    exit(1);
  }

  close(fd);
  exit(0);
}
