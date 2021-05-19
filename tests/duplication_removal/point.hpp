#pragma once
#include <cstdint>
#include <functional>
//
#include <lyrahgames/xstd/pun_cast.hpp>

struct point {
  // Default Ordering
  constexpr auto operator<=>(const point&) const noexcept = default;
  // State
  float x, y, z;
};

namespace std {
template <>
struct hash<point> {
  size_t operator()(const point& p) const noexcept {
    using lyrahgames::xstd::pun_cast;
    size_t a = pun_cast<uint32_t>(p.z);
    size_t b = pun_cast<uint32_t>(p.y);
    size_t c = pun_cast<uint32_t>(p.x);
    return (a << 11) ^ (b << 5) ^ c;
  }
};
}  // namespace std