#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int read_line(char *buf, int max_len) {
    int i = 0;
    while (i < max_len) {
        // 一次读取一个字符
        if (read(0, buf + i, 1) <= 0) {
            // EOF或错误
            if (i == 0) return 0;  // 没有数据，EOF
            break;
        }
        
        if (buf[i] == '\n') {
            break;  // 读取到换行符，结束当前行
        }
        i++;
    }
    
    buf[i] = '\0';  // 添加字符串结束符
    return i;
}

int parse_args(char *line, char *argv[], int argc) {
    char *p = line;
    int i = argc;  // 从现有参数数量开始追加
    
    while (*p && i < MAXARG - 1) {
        // 跳过前导空格
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        
        if (*p == '\0') break;  // 字符串结束
        
        argv[i++] = p;  // 记录参数起始位置
        
        // 查找参数结束位置
        while (*p && *p != ' ' && *p != '\t') {
            p++;
        }
        
        // 如果没有结束，添加字符串结束符，并移动指针
        if (*p) {
            *p++ = '\0';
        }
    }
    
    argv[i] = 0;  // 参数数组以NULL结尾
    return i;
}

int main(int argc, char *argv[]) {
    char buf[512];
    char *xargs_argv[MAXARG];
    int xargs_argc;
    
    // 检查命令行参数
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }
    
    // 复制原始命令及其参数
    for (int i = 1; i < argc; i++) {
        xargs_argv[i - 1] = argv[i];
    }
    xargs_argc = argc - 1;
    
    // 逐行读取标准输入，并执行命令
    while (read_line(buf, sizeof(buf)) > 0) {
        // 解析当前行作为附加参数
        xargs_argc = parse_args(buf, xargs_argv, xargs_argc);
        
        // 创建子进程执行命令
        int pid = fork();
        if (pid < 0) {
            fprintf(2, "xargs: fork failed\n");
            exit(1);
        } else if (pid == 0) {
            // 子进程执行命令
            if (exec(xargs_argv[0], xargs_argv) < 0) {
                fprintf(2, "xargs: exec %s failed\n", xargs_argv[0]);
                exit(1);
            }
        } else {
            // 父进程等待子进程完成
            wait(0);
        }
        
        // 重置参数数量为初始命令参数
        xargs_argc = argc - 1;
    }
    
    exit(0);
}
