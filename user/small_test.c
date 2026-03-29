#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main(void) {
  struct procinfo buf[1];
  int r = ps_listinfo(buf, 1);

  if(r == -2){
    printf("small_test: ok (buffer too small)\n");
    exit(0);
  }

  fprintf(2, "small_test: expected -2, got %d\n", r);
  exit(1);
}