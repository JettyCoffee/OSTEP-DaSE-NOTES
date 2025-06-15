// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct bucket {
  struct spinlock lock; // 自旋锁，用于保护对🪣的并发访问
  struct buf head; // 缓冲区指针，用于构建一个链表，实现LRU缓冲区排序
  // 链表通过prev和next指针连接，head.next指向最近使用的缓冲区，head.prev指向最久未使用的缓冲区
};

struct {
  struct buf buf[NBUF]; // 数组每个元素代表一个缓冲区，buf结构体储存了实际的数据和控制信息
  struct bucket buckets[NBUCKETS]; // 数组每个元素代表一个桶，桶的数量由NBUCKETS定义
} bcache;

static uint hash_v(uint key) {
  return key % NBUCKETS; // 计算哈希值，桶的数量由NBUCKETS定义
}

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

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  uint v = hash_v(blockno);
  struct bucket* bucket = &bcache.buckets[v];
  acquire(&bucket->lock); // 请求桶的锁，保护桶内缓冲区的并发访问

  for (struct buf *buf = bucket->head.next; buf != &bucket->head;
       buf = buf->next) { // 遍历桶内的缓冲区
    if(buf->dev == dev && buf->blockno == blockno){ // 找到匹配的缓冲区
      buf->refcnt++; // 引用计数加1
      release(&bucket->lock); // 释放桶的锁
      acquiresleep(&buf->lock); // 请求缓冲区的锁，保护缓冲区内数据的并发访问
      return buf;
    }
  }

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
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
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


