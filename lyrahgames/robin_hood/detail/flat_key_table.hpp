#pragma once
#include <algorithm>
#include <concepts>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
//
#include <lyrahgames/robin_hood/meta.hpp>
//
#include <lyrahgames/robin_hood/detail/basic_iterator.hpp>
#include <lyrahgames/robin_hood/detail/flat_key_table_base.hpp>

namespace lyrahgames::robin_hood::detail {

template <generic::key Key, generic::allocator Allocator = std::allocator<Key>>
struct flat_key_table
    : public flat_key_table_base<Key, Allocator>,
      public basic_iterator_interface<flat_key_table<Key, Allocator>> {
  using base = flat_key_table_base<Key, Allocator>;
  using typename base::allocator;
  using typename base::key_type;
  using typename base::psl_type;
  using typename base::size_type;

  using iterator_interface =
      basic_iterator_interface<flat_key_table<Key, Allocator>>;
  using typename iterator_interface::const_iterator;
  using typename iterator_interface::iterator;

  using base::construct_key;
  using base::destroy_key;
  using base::keys;
  using base::psl;

  using base::base;
  using base::swap;

  bool empty(size_type index) const noexcept { return psl[index] == 0; }

  auto entry(size_type index) noexcept -> const key_type& {
    return keys[index];
  }

  auto entry(size_type index) const noexcept -> const key_type& {
    return keys[index];
  }

  void move_construct(size_type index, size_type from) {
    construct_key(index, std::move(keys[from]));
  }

  void move_construct(size_type index, iterator it) {
    construct_key(index, std::move(it.base->keys[it.index]));
  }

  void move_assign(size_type index, iterator it) {
    keys[index] = std::move(it.base->keys[it.index]);
  }

  auto index_iterator(size_type index) noexcept {
    return iterator{this, index};
  }

  void destroy(size_type index) noexcept {
    destroy_key(index);
    psl[index] = 0;
  }

  void move(size_type to, size_type from) { keys[to] = std::move(keys[from]); }

  void swap(size_type first, size_type second) noexcept {
    // With this, we can use custom swap routines when they are defined as
    // member functions. Otherwise, we try to use the standard.
    using xstd::swap;
    swap(keys[first], keys[second]);
  }
};

template <generic::key Key, generic::allocator Allocator>
std::ostream& operator<<(std::ostream&                         os,
                         const flat_key_table<Key, Allocator>& table) {
  using namespace std;
  os << '\n';
  for (size_t i = 0; i < table.size; ++i) {
    os << setw(15) << i;
    if (!table.valid(i)) {
      os << ' ' << setfill('-') << setw(45) << '\n' << setfill(' ');
      continue;
    }
    os << setw(15) << table.keys[i] << setw(15) << table.psl[i] << '\n';
  }
  return os;
}

}  // namespace lyrahgames::robin_hood::detail
