#pragma once

#include <array>
#include <cstdint>

uint64_t getNonce(const std::array<uint8_t, 32>& seed, uint32_t difficulty,
                  uint64_t& checkedNoncesCount);
