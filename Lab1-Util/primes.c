#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ 0   // 管道读取端索引
#define WRITE 1  // 管道写入端索引

int main(int argc, char *argv[]) {
    int p[2];
    pipe(p);
    
    if (fork() > 0) {
        close(p[READ]);  // 父进程只写入，不需要读取端
        
        // 向管道写入2到35的所有整数
        for (int i = 2; i <= 35; i++) {
            write(p[WRITE], &i, sizeof(i));
        }
        
        close(p[WRITE]);  // 没有其他数据写入需求，关闭写入端
        wait(0);
        exit(0);
    }
    
    // 以下是筛选质数的过程，每个进程处理一个质数
    int left_read = p[READ];  // 保存当前读取端
    close(p[WRITE]);  // 子进程只读取，关闭写入端
    
    int prime;  // 存储当前找到的质数
    int right_pipe[2];  // 用于向下一个进程传递数据的管道
    
    while (read(left_read, &prime, sizeof(prime)) > 0) {
        printf("prime %d\n", prime);  // 输出找到的质数(注意与grade-lab-util所需格式相同)
        
        pipe(right_pipe);  // 创建新管道用于传递筛选后的数据
        
        if (fork() == 0) {
            // 子进程将成为下一个筛选器
            close(right_pipe[WRITE]);  // 子进程只读取
            close(left_read);  // 关闭旧管道读取端，防止堵塞
            left_read = right_pipe[READ];  // 更新左管道为右管道
            continue;
        }
        
        // 父进程筛选数据并传递给子进程
        close(right_pipe[READ]);  // 父进程只写入
        int num;
        
        // 读取所有剩余数字并筛选
        while (read(left_read, &num, sizeof(num)) > 0) {
            if (num % prime != 0) {
                write(right_pipe[WRITE], &num, sizeof(num));  // 如果不能被当前质数整除，则传递给下一个进程
            }
        }
        
        close(left_read);
        close(right_pipe[WRITE]);
        
        wait(0);
        exit(0);
    }
    
    close(left_read);
    exit(0);
}