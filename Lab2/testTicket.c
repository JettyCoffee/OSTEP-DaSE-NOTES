#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc != 2) {
    printf(1, "Usage: testTicket <number>\n");
    exit();
  }
  
  int tickets = atoi(argv[1]);
  printf(1, "Process %d: Setting tickets to %d\n", getpid(), tickets);
  
  if(settickets(tickets) < 0) {
    printf(1, "Error: settickets(%d) failed\n", tickets);
    exit();
  }

  int i = 0;
  while(1) {
    i++;
  }
  exit();
}