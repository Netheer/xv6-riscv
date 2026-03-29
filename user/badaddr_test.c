#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int main(void) {
  struct procinfo *bad = (struct procinfo *)1;
  int r = ps_listinfo(bad, 10);

  if(r < 0 && r != -2){
    printf("badaddr_test: ok (invalid address), code=%d\n", r);
    exit(0);
  }

  fprintf(2, "badaddr_test: expected invalid-address error, got %d\n", r);
  exit(1);
}