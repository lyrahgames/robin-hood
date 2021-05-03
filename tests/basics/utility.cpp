#include <doctest/doctest.h>
//
#include <lyrahgames/robin_hood/utility.hpp>

using namespace std;
using namespace lyrahgames::robin_hood;

TEST_CASE("32-Bit Unsigned Integer Binary Logarithm") {
  CHECK(log2(uint32_t{0}) == uint32_t{0});
  CHECK(log2(uint32_t{1}) == uint32_t{0});
  CHECK(log2(uint32_t{2}) == uint32_t{1});

  for (uint32_t i = 0; i < 20; ++i) {
    const auto start = uint32_t{1} << i;
    const auto end = start << 1;
    for (uint32_t j = start; j < end; ++j)
      CHECK(log2(j) == i);
  }
}

TEST_CASE("64-Bit Unsigned Integer Binary Logarithm") {
  CHECK(log2(uint64_t{0}) == uint64_t{0});
  CHECK(log2(uint64_t{1}) == uint64_t{0});
  CHECK(log2(uint64_t{2}) == uint64_t{1});

  for (uint64_t i = 0; i < 20; ++i) {
    const auto start = uint64_t{1} << i;
    const auto end = start << 1;
    for (uint64_t j = start; j < end; ++j)
      CHECK(log2(j) == i);
  }
}

TEST_CASE("32-Bit Ceiling to Power of Two") {
  CHECK(ceil_pow2(uint32_t{0}) == uint32_t{0});
  CHECK(ceil_pow2(uint32_t{1}) == uint32_t{2});
  CHECK(ceil_pow2(uint32_t{2}) == uint32_t{2});
  CHECK(ceil_pow2(uint32_t{3}) == uint32_t{4});
  CHECK(ceil_pow2(uint32_t{4}) == uint32_t{4});
  CHECK(ceil_pow2(uint32_t{5}) == uint32_t{8});

  for (uint32_t i = 2; i < (uint32_t{1} << 20); i <<= 1) {
    CHECK(ceil_pow2(i) == i);
    for (uint32_t j = i + 1; j < (i << 1); ++j)
      CHECK(ceil_pow2(j) == (i << 1));
  }
}

TEST_CASE("32-Bit Ceiling to Power of Two") {
  CHECK(ceil_pow2(uint64_t{1}) == uint64_t{2});
  CHECK(ceil_pow2(uint64_t{2}) == uint64_t{2});
  CHECK(ceil_pow2(uint64_t{3}) == uint64_t{4});
  CHECK(ceil_pow2(uint64_t{4}) == uint64_t{4});
  CHECK(ceil_pow2(uint64_t{5}) == uint64_t{8});

  for (uint64_t i = 2; i < (uint64_t{1} << 20); i <<= 1) {
    CHECK(ceil_pow2(i) == i);
    for (uint64_t j = i + 1; j < (i << 1); ++j)
      CHECK(ceil_pow2(j) == (i << 1));
  }
}