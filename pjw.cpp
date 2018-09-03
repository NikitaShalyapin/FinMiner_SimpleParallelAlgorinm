#include "pjw.h"

uint32_t pjw(const uint8_t* data, uint64_t size)
{
  uint32_t hash = 0;
  uint32_t test = 0;

  for (uint64_t i = 0; i < size; i++) {
    hash = (hash << 4) + data[i];
    if ((test = hash & 0xf0000000) != 0) {
      hash = ((hash ^ (test >> 24)) & (0xfffffff));
    }
  }
  return hash;
}
