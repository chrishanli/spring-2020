// pseudo_random.h    09/03/2020
// J_Code
// brief: ����ȷ�����ӵ�α�����

#include <inttypes.h>

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295
#endif

void setSeed(uint64_t seed);
uint32_t lcg64_temper(void);
