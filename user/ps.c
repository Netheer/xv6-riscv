#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

static char* state_to_text(int state) {
  switch(state){
  case PS_UNUSED:   return "UNUSED";
  case PS_USED:     return "USED";
  case PS_SLEEPING: return "SLEEPING";
  case PS_RUNNABLE: return "RUNNABLE";
  case PS_RUNNING:  return "RUNNING";
  case PS_ZOMBIE:   return "ZOMBIE";
  default:          return "UNKNOWN";
  }
}

int main(void) {
  int cap = ps_listinfo(0, 0);
  if(cap < 0){
    fprintf(2, "ps: ps_listinfo(count) failed: %d\n", cap);
    exit(1);
  }

  if(cap == 0)
    cap = 4;

  for(;;){
    struct procinfo *buf =
      (struct procinfo *)malloc(cap * sizeof(struct procinfo));
    if(buf == 0){
      fprintf(2, "ps: malloc failed\n");
      exit(1);
    }

    int n = ps_listinfo(buf, cap);
    if(n >= 0){
      printf("PID\tPPID\tSTATE       NAME\n");
      for(int i = 0; i < n; i++){
        printf("%d\t%d\t%s    %s\n",
               buf[i].pid,
               buf[i].parent_pid,
               state_to_text(buf[i].state),
               buf[i].name);
      }
      free(buf);
      exit(0);
    }

    free(buf);

    if(n == -2){
      if(cap < 4)
        cap = 4;
      else
        cap *= 2;
      continue;
    }

    fprintf(2, "ps: ps_listinfo failed: %d\n", n);
    exit(1);
  }
}