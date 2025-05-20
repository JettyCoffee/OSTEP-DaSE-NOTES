#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(void)
{
  struct pstat ps;
  int i;
  
  if(getpinfo(&ps) < 0) {
    printf(1, "Error: getpinfo() failed\n");
    exit();
  }
  
  int total_ticks = 0;
  
  printf(1, "------------ Initially assigned Tickets = 5 ----------\n");
  printf(1, "ProcessID Runnable(0/1) Tickets Ticks\n");
  
  // 计算总时间片
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i]) {
      total_ticks += ps.ticks[i];
    }
  }
  
  // 打印进程信息
  for(i = 0; i < NPROC; i++) {
    if(ps.inuse[i]) {
      printf(1, "%d %d %d %d\n", 
        ps.pid[i], 
        ps.inuse[i], 
        ps.tickets[i], 
        ps.ticks[i]);
    }
  }
  
  printf(1, "------------------- Total Ticks = %d -----------------\n", total_ticks);
  
  exit();
}