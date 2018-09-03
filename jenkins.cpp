#include "jenkins.h"

uint32_t jenkins(const uint8_t* data, uint64_t size)
{
  uint32_t hash = 0;
  for (uint64_t i = 0; i < size; i++) {
    hash += data[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}
