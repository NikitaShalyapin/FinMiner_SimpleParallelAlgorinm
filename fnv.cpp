#include "fnv.h"

static const uint32_t FNV_32_PRIME = 0x01000193;

uint32_t fnv(const uint8_t* data, uint64_t size)
{
  uint32_t hval = 0x811c9dc5;

  for (uint64_t i = 0; i < size; i++) {
    hval *= FNV_32_PRIME;
    hval ^= (uint32_t)data[i];
  }
  return hval;
}
