#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace lyrahgames::robin_hood {

inline uint32_t log2(const uint32_t x) {
  uint32_t y;
  asm("\tbsr %1, %0\n" : "=r"(y) : "r"(x));
  return y;
}

inline uint64_t log2(uint64_t x) noexcept {
  uint64_t y;
  asm("\tbsr %1, %0\n" : "=r"(y) : "r"(x));
  return y;
}

/// Return smallest positive integral power of two
/// that is bigger or equal to the given positive number.
/// If the given number is zero the function returns zero.
/// The function fails if the answer would be 2^32.
/// With NDEBUG defined, it would also return zero.
inline uint32_t ceil_pow2(uint32_t x) {
  assert(x <= (uint32_t{1} << 31));
  return uint64_t{1} << (log2(x - 1) + 1);
}

/// Return smallest positive integral power of two
/// that is bigger or equal to the given positive number.
/// The function fails if the given number is zero or if the answer would be
/// 2^64. With NDEBUG defined, undefined behavior is the result in such cases.
inline uint64_t ceil_pow2(uint64_t x) {
  assert((0 < x) && (x <= (uint64_t{1} << 63)));
  return uint64_t{1} << (log2(x - 1) + 1);
}

}  // namespace lyrahgames::robin_hood