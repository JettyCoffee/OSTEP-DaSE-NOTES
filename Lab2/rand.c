#include "types.h"
#include "defs.h"
#include "rand.h"

// 线性同余伪随机数生成器参数
#define A 1664525
#define C 1013904223
#define M 0xFFFFFFFF

static uint seed = 1;

// 初始化随机数生成器种子
void rand_init(void)
{
  seed = 1;
}

// 生成下一个随机数
static uint next_rand(void)
{
  seed = (A * seed + C) & M;
  return seed;
}

// 返回范围在[0, max]内的伪随机数
uint random_at_most(uint max)
{
  if (max == 0)
    return 0;
    
  // 生成一个0到max之间的随机数
  return next_rand() % (max + 1);
}