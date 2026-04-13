# OSTEP-DaSE-NOTES

## 项目简介

本仓库包含了DaSE（数据科学与工程学院）操作系统导论课程的完整学习资料，包括：
- 📚 课程笔记和理论学习资料
- 💻 实验代码和实现
- 📄 相关PDF文档和参考资料

## 目录结构

```
├── PPT&Notes/              # 课程PPT和学习笔记
│   ├── os-1.pdf ~ os-17-2.pdf # 课程PPT文件
│   ├── OS2025 Notes.md        # 课程笔记（Markdown格式）
│   └── OS2025 Notes.pdf       # 课程笔记（PDF格式）
│
├── Lab1-Util/             # 实验一：工具程序
│   ├── 实验一 Util.pdf        # 实验指导书
│   ├── sleep.c               # sleep程序实现
│   ├── pingpong.c            # pingpong程序实现
│   ├── primes.c              # primes程序实现
│   ├── find.c                # find程序实现
│   ├── xargs.c               # xargs程序实现
│   └── OS Lab1实验报告.*      # 实验报告
│
├── Lab2-彩排调度/         # 实验二：彩票调度
│   ├── 实验二 彩票调度.pdf    # 实验指导书
│   ├── proc.c, proc.h        # 进程相关实现
│   ├── syscall.c, syscall.h  # 系统调用实现
│   ├── testTicket.c          # 彩票调度测试程序
│   └── OS Lab2实验报告.*      # 实验报告
│
├── Lab3-页表/             # 实验三：页表管理
│   ├── 实验三 页表.pdf        # 实验指导书
│   ├── vm.c                  # 虚拟内存管理实现
│   ├── exec.c                # 进程执行相关
│   └── OS Lab3实验报告.*      # 实验报告
│
├── Lab4-Locks/            # 实验四：锁机制
│   ├── 实验四 Locks.pdf       # 实验指导书
│   ├── bio.c                 # 缓冲区I/O实现
│   └── OS Lab4实验报告.*      # 实验报告
│
├── 参考资料/
│   ├── Advanced Programming in the UNIX Environment 3rd Edition.pdf
│   ├── UNIX环境高级编程（中文第三版）.pdf
│   └── 环境配置—QEMU+xv6.pdf
│
└── res/                   # 其他资源文件
    └── dsst2025/             # 数据结构相关资料
```

## 🚀 实验内容

### Lab1 - Util 工具程序
- **sleep**: 暂停指定时间的程序
- **pingpong**: 父子进程间通信程序
- **primes**: 并发素数筛选程序
- **find**: 文件查找工具
- **xargs**: 命令行参数处理工具

### Lab2 - 彩票调度 (Lottery Scheduling)
- 实现基于彩票的进程调度算法
- 添加系统调用获取进程信息
- 测试调度算法的公平性

### Lab3 - 页表管理 (Page Tables)
- 实现虚拟内存管理机制
- 页表映射和地址转换
- 内存保护和权限管理

### Lab4 - 锁机制 (Locks)
- 实现并发控制机制
- 缓冲区锁管理
- 解决竞态条件问题

## 开发环境

本项目基于 **xv6** 操作系统进行开发，使用 **QEMU** 模拟器运行。

### 环境配置
详细的环境配置步骤请参考：`环境配置—QEMU+xv6.pdf`

### 编译和运行
```bash
# 在相应的Lab目录下
make
make qemu
```
---
