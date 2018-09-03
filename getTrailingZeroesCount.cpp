#include "getTrailingZeroesCount.h"

uint32_t getTrailingZeroesCount(uint32_t x)
{
  uint32_t z = 0;
  for (int i = 0; i < 32; i++) {
    if (x & 1) {
      break;
    }
    z++;
    x >>= 1;
  }
  return z;
}
