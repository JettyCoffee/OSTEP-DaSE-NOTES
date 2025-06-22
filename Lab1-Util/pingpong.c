#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NULL ((void*)0)  // 手动定义 NULL
#define MSGSIZE 16

int main(void) {
    int fd[2];
    char buf[MSGSIZE];
    pipe(fd);
    int pid = fork();
    if (pid > 0){
        write(fd[1],"ping",MSGSIZE);
        wait(NULL);
        read(fd[0], buf, MSGSIZE);
        fprintf(2,"%d: received %s\n",getpid(), buf);
    } else {
        read(fd[0], buf, MSGSIZE);
        printf("%d: received %s\n",getpid(), buf);
        write(fd[1],"pong", MSGSIZE);
    }
    exit(0);
}