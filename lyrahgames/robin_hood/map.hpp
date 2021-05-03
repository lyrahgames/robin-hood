#pragma once
#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
//
#include <iomanip>
#include <iostream>

namespace lyrahgames::robin_hood {

/// \class map map.hpp lyrahgames/robin_hood/map.hpp
template <typename key_type,
          typename mapped_type,
          typename hasher = std::hash<key_type>,
          typename equality = std::equal_to<key_type>>
class map {
  template <bool constant>
  struct basic_iterator;
  struct container;

 public:
  using real = float;
  using size_type = size_t;
  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;

  friend std::ostream& operator<<(std::ostream& os, const map& m) {
    using namespace std;
    for (size_t i = 0; i < m.table.size; ++i)
      os << setw(15) << m.table.keys[i] << setw(15) << m.table.values[i]
         << setw(15) << m.table.psl[i] << '\n';
    return os;
  }

  bool empty() const { return load == 0; }
  auto size() const { return load; }
  auto capacity() const { return table.size; }
  auto load_factor() const { return real(size()) / capacity(); }
  auto max_load_factor() const { return max_load; }

  // real -> open_normalized<real>
  void set_max_load_factor(real x);

  auto begin() noexcept -> iterator;
  auto begin() const noexcept -> const_iterator;
  auto end() noexcept -> iterator;
  auto end() const noexcept -> const_iterator;

  auto find(const key_type& key) noexcept -> iterator;
  auto find(const key_type& key) const noexcept -> const_iterator;

  auto operator[](const key_type& key) -> mapped_type&;
  auto operator()(const key_type& key) -> mapped_type&;
  auto operator()(const key_type& key) const -> const mapped_type&;

  bool erase(const key_type& key) noexcept;

  void rehash();
  void resize();

 private:
  auto new_key_swap_index(const key_type& key) const noexcept
      -> std::pair<size_t, size_t>;
  void insert_key_by_swap(const key_type& key, size_t index, size_t psl);
  void double_capacity_and_rehash();
  void erase_by_swap(size_t index);

  container table{8};
  hasher hash{};
  equality equal{};
  size_type load{};
  // open_normalized<real>
  real max_load{0.8};
  // We need stats:
  // min, max index -> faster begin() and end()
  // min, max psl
};

}  // namespace lyrahgames::robin_hood

#include <lyrahgames/robin_hood/map.ipp>