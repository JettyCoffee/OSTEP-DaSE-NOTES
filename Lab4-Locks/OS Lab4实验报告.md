# **华东师范大学数据学院上机实践报告**

# OS Lab4 Locks - Buffer Cache 优化

| 课程名称： 操作系统     | 年级：2023            | 上机实践成绩：            |
| :---------------------- | --------------------- | ------------------------- |
| **指导老师**：翁楚良    | **姓名**：陈子谦      |                           |
| **上机实践名称**：Locks | **学号**：10235501454 | **上机实践日期**：25.5.23 |

## 一、实验目的

优化xv6操作系统中的缓冲区缓存（buffer cache），减少多个进程之间对缓冲区缓存锁的争用，从而提高系统的性能和并发能力。通过设计和实现一种更加高效的缓冲区管理机制，使得不同进程可以更有效地使用和管理缓冲区，减少锁竞争和性能瓶颈。

### 科学价值与相关研究

在多核操作系统中，缓冲区缓存是文件系统性能的关键瓶颈之一。传统的单一全局锁机制在高并发场景下会导致严重的锁竞争，限制系统的可扩展性。本实验通过实现细粒度锁机制，解决了这一经典的并发控制问题，为理解现代操作系统中的并发优化技术提供了重要的实践基础。

相关研究表明，采用分段锁（segmented locking）和哈希分区技术可以显著提升多核系统的并发性能，这在数据库系统、文件系统等领域得到了广泛应用。

## 二、实验内容

### 问题定义

**语言描述：** 原始 xv6 系统中，所有缓冲区由单一的 `bcache.lock` 保护，多个进程同时访问不同块时仍需竞争同一把锁，导致不必要的串行化和性能瓶颈。

**数学形式：** 设系统中有 n 个进程同时进行磁盘 I/O 操作，在原始实现中，锁竞争的时间复杂度为 O(n)，而优化后的哈希桶机制可将其降低到 O(n/k)，其中 k 为哈希桶的数量。

### 主要内容

1. **数据结构重新设计：** 引入哈希桶机制，将缓冲区按块号分散到不同的桶中
2. **锁粒度细化：** 将单一全局锁替换为多个桶级别的锁
3. **缓冲区分配策略优化：** 采用原子操作管理缓冲区使用状态
4. **LRU 链表管理：** 在每个桶内独立维护 LRU 链表

## 三、使用环境

- **操作系统：** Linux 
- **开发环境：** xv6-labs-2021
- **编译器：** RISC-V GCC 工具链
- **测试平台：** QEMU RISC-V 模拟器
- **并发测试：** 多核心环境（支持最多 8 个 CPU）

## 四、实验过程和结果

1. **了解缓冲区缓存：**阅读相关文档和代码，了解xv6中的缓冲区缓存的工作原理、数据结构以及锁机制。

2. **运行测试并分析问题：**运行`bcachetest`测试程序，观察锁竞争情况和测试结果。测试结果表明，在多个进程之间，对bcache.lock锁的竞争比较激烈，导致多个进程在试图获取该锁时需要进行较多次的test-and-set操作和acquire()操作。这说明了缓冲区管理中存在较大的竞争问题，可能影响了系统的性能和响应速度。

3. **优化缓冲区管理：**根据分析结果，我们需要重新设计和实现缓冲区管理机制：

   ```c
   struct bucket {
     struct spinlock lock; // 自旋锁，用于保护对🪣的并发访问
     struct buf head; // 缓冲区指针，用于构建一个链表，实现LRU缓冲区排序
     // 链表通过prev和next指针连接，head.next指向最近使用的缓冲区，head.prev指向最久未使用的缓冲区
   };
   ```

   **struct bucket:** 这个结构体用于实现缓冲区的哈希分桶机制。每个分桶都包含一个链表，用于存储缓冲区。

   - `struct spinlock lock`是自旋锁，用于保护对该分桶的并发访问。当有线程要操作这个分桶时，需要先获取这个锁。
   - `struct buf head`是缓冲区指针，用于构建一个链表，以实现最近使用的缓冲区排序。链表通过 `prev` 和 `next` 字段连接，`head.next` 指向最近使用的缓冲区，`head.prev` 指向最不常使用的缓冲区。

   通过哈希函数将缓冲区映射到不同的分桶中，可以减少对整个缓冲区数组的搜索。

   ```c
   struct {
     struct buf buf[NBUF]; // 数组每个元素代表一个缓冲区，buf结构体储存了实际的数据和控制信息
     struct bucket buckets[NBUCKETS]; // 数组每个元素代表一个桶，桶的数量由NBUCKETS定义
   } bcache;
   ```

   ```c
   param.h:
   #define MAXPATH      128   // maximum file path name
   #define NBUCKETS     13   // number of buckets in the buffer cache
   ```

   **全局 bcache 结构体:** 整个缓冲区管理的主要容器。

   - `struct buf buf[NBUF]`数组的每个元素代表一个缓冲区。缓冲区结构体 `struct buf` 存储了实际的数据内容、元数据和与缓冲区相关的控制信息。
   - `struct bucket bucket[NBUCKET]`数组的每个元素代表一个分桶。每个分桶通过上述 `struct bucket` 结构体来实现。

   全局结构体通过将缓冲区数组和分桶数组结合在一起，实现了对缓冲区的高效管理、分配和释放。它允许对每个分桶进行独立的加锁，从而减少了不同分桶之间的竞争，并且通过哈希函数将缓冲区分散存储在不同的分桶中，提高了访问效率。

   此外，配合哈希函数 `hash_v`，其输出将被用作缓冲区管理中的分桶索引，从而将不同的块号分散到不同的分桶中。

   ```c
   static uint hash_v(uint key) {
     return key % NBUCKETS; // 计算哈希值，桶的数量由NBUCKETS定义
   }
   ```

4. **初始化缓冲区管理所需的数据结构：**对每个缓冲区进行初始化：

   - 使用 `initsleeplock` 函数初始化缓冲区的休眠锁（用于同步访问缓冲区数据），并为每个缓冲区设置一个描述性的名字 "buffer"。
   - 对每个分桶进行初始化：为每个分桶结构体进行初始化，确保分桶的自旋锁和链表头正确初始化。

   ```c
   void
   binit(void)
   {
     for(int i = 0; i < NBUF; i++){
       initsleeplock(&bcache.buf[i].lock, "buffer"); 
     }
     for(int i = 0; i < NBUCKETS; i++){
       initlock(&bcache.buckets[i].lock, "bcache.bucket");
       bcache.buckets[i].head.prev = &bcache.buckets[i].head;
       bcache.buckets[i].head.next = &bcache.buckets[i].head;
       // 初始化每个桶链表的 head，使其成为一个空的循环链表
     }
   }
   ```

5. **实现了缓冲区的获取（bget）逻辑：**`bget`函数用于从缓冲区中查找指定设备和块号对应的缓冲区：

   - 使用哈希函数将块号映射到一个分桶中。
   - 根据计算得到的分桶索引 `v`，获取对应的分桶结构体的指针 `bucket`。
   - 获取分桶的自旋锁，确保在进行缓冲区操作时，只有一个线程能够访问该分桶。

   ```c
   uint v = hash_v(blockno);
   struct bucket* bucket = &bcache.buckets[v];
   acquire(&bucket->lock); // 请求桶的锁，保护桶内缓冲区的并发访问
   ```

   - 查找已缓存的块： 遍历查找是否已经缓存了指定设备和块号的缓冲区。如果找到，则该缓冲区引用计数增加，释放分桶的自旋锁，获取缓冲区的休眠锁，并返回该缓冲区。
   - 如果块未被缓存，则从未使用的缓冲区中选择一个进行分配。
     - 遍历找到一个未使用的缓冲区，使用原子操作标记为已使用。
     - 设置该缓冲区的设备、块号、有效性、引用计数等信息。
     - 将该缓冲区插入分桶的链表头，表示最近使用，更新链表头的 `prev` 指针。
   - 释放分桶的自旋锁，获取该缓冲区的休眠锁，并返回该缓冲区。
   - 如果没有可用的缓冲区，则无法分配新的缓冲区。

   ```c
     for (int i = 0; i < NBUF; ++i) {
       // 如果块未被缓存，则从未使用的缓冲区中选择一个进行分配
       if (!bcache.buf[i].used && 
           !__atomic_test_and_set(&bcache.buf[i].used, __ATOMIC_ACQUIRE)) {
         struct buf *buf = &bcache.buf[i];
         buf->dev = dev;
         buf->blockno = blockno;
         buf->valid = 0;
         buf->refcnt = 1; 
   
         buf->next = bucket->head.next; // 将新缓冲区插入到桶的头部
         buf->prev = &bucket->head; // 更新新缓冲区的前后指针
         bucket->head.next->prev = buf;
         bucket->head.next = buf;
         release(&bucket->lock);
         acquiresleep(&buf->lock);
         return buf;
       }
   ```

6. **修改释放缓冲区（brelse）逻辑：**

   - 检查以确保只有拥有缓冲区休眠锁的线程才能释放缓冲区。
   - 释放缓冲区的休眠锁，以允许其他线程访问该缓冲区。
   - 根据缓冲区的块号计算哈希索引 `v`，获取对应的分桶指针 `bucket`。
   - 获取分桶的自旋锁，防止操作时被其他线程干扰。
   - 将缓冲区的引用计数减一。
   - 如果缓冲区的引用计数变为零，表示没有线程在等待该缓冲区，可以从链表中移除。
     - 更新前后缓冲区的链接，将该缓冲区从链表中移除。
     - 使用原子操作清除缓冲区的 "used" 标志，表示该缓冲区未被使用。
   - 操作结束释放分桶的自旋锁。

   ```c
   void
   brelse(struct buf *b)
   {
     if(!holdingsleep(&b->lock))
       panic("brelse");
   
     releasesleep(&b->lock);
   
     uint v = hash_v(b->blockno);
     struct bucket* bucket = &bcache.buckets[v];
     acquire(&bucket->lock); // 获取分桶的自旋锁，防止操作时被其他线程干扰
     b->refcnt--; // 将缓冲区的引用计数减一
     // 如果缓冲区的引用计数变为零，表示没有线程在等待该缓冲区，可以从链表中移除
     if (b->refcnt == 0) {
       b->next->prev = b->prev;
       b->prev->next = b->next;
       __atomic_clear(&b->used, __ATOMIC_RELEASE); // 使用原子操作清除缓冲区的 "used" 标志，表示该缓冲区未被使用
     }
     
     release(&bucket->lock); // 释放桶的锁
   }
   ```

   ```c
   struct buf {
     char used; // 0: not used, 1: used
   };
   ```

7. **修改增减缓冲区计数函数的逻辑：**在增减缓冲区计数之前，根据缓冲区的块号计算哈希索引 `v`，获取对应的分桶结构体指针 `bucket`和分桶的自旋锁，以确保在操作缓冲区引用计数时不会被其他线程干扰。

   ```c
   void
   bpin(struct buf *b) {
     uint v = hash_v(b->blockno);
     struct bucket* bucket = &bcache.buckets[v];
     acquire(&bucket->lock);
     b->refcnt++;
     release(&bucket->lock);
   }
   
   void
   bunpin(struct buf *b) {
     uint v = hash_v(b->blockno);
     struct bucket* bucket = &bcache.buckets[v];
     acquire(&bucket->lock);
     b->refcnt--;
     release(&bucket->lock);
   }
   ```

8. **运行bcachetest**：运行修改后的bcachetest测试程序，观察锁竞争情况和测试结果。优化后，锁竞争应该大幅减少。

   ![Snipaste_2025-05-22_22-04-53](E:\OS 2025\lab4\assets\Snipaste_2025-05-22_22-04-53.jpg)

   比较实验开始前和实验进行后的测试结果，可以看出锁竞争明显减少，在实验进行之前的测试中，锁竞争非常严重。`bcachetest` 测试中显示了大量的 `test-and-set` 操作和锁的获取次数，这表明在并发访问缓冲区池时存在大量的竞争。而最新测试结果中，锁竞争明显减少，`test-and-set` 操作和锁的获取次数较少。

   且在实验进行后的测试结果中，`test0` 测试显示的锁竞争情况也相对较低，并发访问时的性能和锁竞争情况得到了改善。

9. **运行usertests**：运行usertests测试程序，确保修改不会影响其他部分的正常运行。

   ![Snipaste_2025-05-23_23-54-54](E:\OS 2025\lab4\assets\Snipaste_2025-05-23_23-54-54.jpg)

### 实验中遇到的问题和解决方法

**并发一致性问题：** 在修改缓冲区管理代码时，可能会遇到并发情况下的数据一致性问题，如资源分配冲突、竞争条件等。起初我没有修改增减缓冲区计数的函数，使得其可能被其他线程干扰导致出错，后来我增加了锁，根据缓冲区的块号计算哈希索引 `v`，获取对应的分桶结构体指针 `bucket`和分桶的自旋锁，以确保在操作缓冲区引用计数时不会被其他线程干扰。

## 五、结论

在开始实验之前，我深入理解了 Buffer Cache 的结构，包括缓存的大小、缓存块的管理方式以及数据结构等。这为我后续的编码和设计提供了重要的指导。

实验要求的功能涵盖了从缓存的获取、写入到缓存的释放等多个方面。我将实验任务分成不同的阶段，逐步实现和测试每个功能。这样有助于确保每个功能都能够独立正常运行。

在多线程环境下，缓存的并发访问可能导致竞态条件等问题。我使用了适当的同步机制，如自旋锁和信号量，以确保缓存的安全性和一致性。

此外，我注意到了这次设计的内核还包含了LRU（最近最少使用）算法，在Buffer Cache中，LRU算法用于确定哪个缓存块应该被释放，以便为新的数据块腾出空间。LRU算法的核心思想是，最近最少使用的缓存块应该被优先释放，因为它们更有可能在未来不会被再次访问。而在结构体的设计上，`struct bucket` 结构体包含一个锁 `lock` 和一个链表头 `head`。链表头 `head` 表示了一个双向循环链表，用于存储缓存块。这个链表是按照缓存块被使用的时间顺序来排序的，即最近被使用的缓存块位于链表头部，最少使用的位于链表尾部。这正是LRU算法的核心思想。
