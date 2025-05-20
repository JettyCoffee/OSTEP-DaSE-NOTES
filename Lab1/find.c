#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* fmtname(char *path)
{
  static char buf[DIRSIZ+1];  // 静态缓冲区存储结果
  char *p;

  // 从路径末尾向前搜索，找到最后一个'/'字符
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;  // 移动到文件名开始位置

  // 返回格式化后的文件名
  if(strlen(p) >= DIRSIZ)
    return p;  // 如果文件名长度超过DIRSIZ，直接返回原文件名
  
  // 否则复制文件名到缓冲区并填充空格
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), 0, DIRSIZ-strlen(p)+1);
  return buf;
}

void find(char *path, char *filename)
{
  char buf[512], *p;  // 缓冲区用于构建完整路径
  int fd;  // 文件描述符
  struct dirent de;  // 目录项结构体
  struct stat st;  // 文件状态结构体

  // 打开目录
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // 获取文件状态
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // 确保path是一个目录
  if(st.type != T_DIR){
    fprintf(2, "find: %s is not a directory\n", path);
    close(fd);
    return;
  }

  // 检查路径长度以防缓冲区溢出
  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }

  // 构建基本路径（以'/'结尾）
  strcpy(buf, path);
  p = buf+strlen(buf);
  *p++ = '/';

  // 读取目录中的所有条目
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    // 跳过无效项
    if(de.inum == 0)
      continue;

    // 构建完整路径（目录+文件名）
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;  // 确保字符串结束

    // 获取文件信息
    if(stat(buf, &st) < 0){
      fprintf(2, "find: cannot stat %s\n", buf);
      continue;
    }

    // 如果是文件且名称匹配，输出路径
    if(st.type == T_FILE && strcmp(de.name, filename) == 0){
      printf("%s\n", buf);
    }
    // 如果是目录且不是"."或".."，递归查找
    else if(st.type == T_DIR && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0){
      find(buf, filename);  // 递归搜索子目录
    }
  }
  
  close(fd);  // 关闭目录
}

int main(int argc, char *argv[])
{
  // 检查参数数量
  if(argc != 3){
    fprintf(2, "Usage: find <directory> <filename>\n");
    exit(1);
  }
  
  find(argv[1], argv[2]);  // 从指定目录开始查找指定文件名
  exit(0);
}
