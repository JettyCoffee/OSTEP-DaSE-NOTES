#ifndef _RAND_H_
#define _RAND_H_

// 返回范围在[0, max]内的伪随机数
uint random_at_most(uint max);

// 初始化随机数生成器
void rand_init(void);

#endif // _RAND_H_