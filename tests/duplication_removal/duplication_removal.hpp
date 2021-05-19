#pragma once
#include <ranges>
#include <vector>
//
#include <unordered_map>
//
#include <lyrahgames/robin_hood/map.hpp>

enum class implementation {
  naive,
  std_unordered_map,
  lyrahgames_robin_hood_map
};

namespace naive {

template <std::ranges::input_range T>
inline auto duplication_count(const T& data) {
  using namespace std;
  size_t result = 0;
  for (auto p = ranges::begin(data); p != ranges::end(data); ++p)
    for (auto q = p + 1; q != ranges::end(data); ++q)
      result += (*p == *q);
  return result;
}

template <std::ranges::input_range T>
inline auto duplication_removal(const T& data) {
  using namespace std;
  using value_type = ranges::range_value_t<T>;

  vector<value_type> result{};
  result.reserve(ranges::size(data));

  for (auto p = ranges::begin(data); p != ranges::end(data); ++p) {
    bool duplicated = false;
    for (auto q = p + 1; q != ranges::end(data); ++q) {
      if (*p != *q) continue;
      duplicated = true;
      break;
    }
    if (!duplicated) result.push_back(*p);
  }

  return result;
}

}  // namespace naive

namespace std_unordered_map {

template <std::ranges::input_range T>
inline auto duplication_count(const T& data) {
  using namespace std;
  using value_type = ranges::range_value_t<T>;

  unordered_map<value_type, int> map{};
  map.rehash(ranges::size(data));
  for (const auto& p : data)
    ++map[p];
  return ranges::size(data) - map.size();
}

template <std::ranges::input_range T>
inline auto duplication_removal(const T& data) {
  using namespace std;
  using value_type = ranges::range_value_t<T>;

  vector<value_type> result{};
  result.reserve(ranges::size(data));

  unordered_map<value_type, int> map{};
  map.rehash(ranges::size(data));
  for (const auto& p : data)
    ++map[p];
  for (const auto& [p, v] : map)
    result.push_back(p);

  return result;
}

}  // namespace std_unordered_map

namespace lyrahgames_robin_hood_map {

template <std::ranges::input_range T>
inline auto duplication_count(const T& data) {
  using namespace std;
  using namespace lyrahgames;
  using value_type = ranges::range_value_t<T>;

  robin_hood::map<value_type, int> map{};
  map.reserve(ranges::size(data));
  for (const auto& p : data)
    ++map[p];
  return ranges::size(data) - map.size();
}

template <std::ranges::input_range T>
inline auto duplication_removal(const T& data) {
  using namespace std;
  using namespace lyrahgames;
  using value_type = ranges::range_value_t<T>;

  vector<value_type> result{};
  result.reserve(ranges::size(data));

  robin_hood::map<value_type, int> map{};
  map.reserve(ranges::size(data));
  for (const auto& p : data)
    ++map[p];
  for (const auto& [p, v] : map)
    result.push_back(p);

  return result;
}

}  // namespace lyrahgames_robin_hood_map