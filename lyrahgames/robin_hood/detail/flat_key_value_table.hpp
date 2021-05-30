#pragma once
#include <algorithm>
#include <concepts>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
//
#include <lyrahgames/xstd/swap.hpp>
//
#include <lyrahgames/robin_hood/meta.hpp>
//
#include <lyrahgames/robin_hood/detail/basic_iterator.hpp>

namespace lyrahgames::robin_hood::detail {

template <generic::key       Key,
          generic::value     Value,
          generic::allocator Allocator = std::allocator<Key>>
struct flat_key_value_table : public basic_iterator_interface<
                                  flat_key_value_table<Key, Value, Allocator>> {
  using size_type  = typename traits::size_type;
  using psl_type   = typename traits::psl_type;
  using key_type   = Key;
  using value_type = Value;
  using entry_type = std::pair<key_type, value_type>;
  using allocator  = Allocator;

  using basic_key_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<key_type>;
  using key_allocator = std::allocator_traits<basic_key_allocator>;

  using basic_psl_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<psl_type>;
  using psl_allocator = std::allocator_traits<basic_psl_allocator>;

  using basic_value_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<value_type>;
  using value_allocator = std::allocator_traits<basic_value_allocator>;

  using iterator =
      basic_iterator<flat_key_value_table<key_type, value_type, allocator>,
                     false>;
  using const_iterator =
      basic_iterator<flat_key_value_table<key_type, value_type, allocator>,
                     true>;

  flat_key_value_table() = default;

  explicit flat_key_value_table(size_type s, allocator a = {})
      : alloc{a}, size{s} {
    init();
  }

  virtual ~flat_key_value_table() noexcept { free(); }

  flat_key_value_table(const flat_key_value_table& t)
      : alloc{t.alloc}, size{t.size} {
    copy(t);
  }

  flat_key_value_table& operator=(const flat_key_value_table& t) {
    free();
    alloc = t.alloc;
    size  = t.size;
    copy(t);
    return *this;
  }

  flat_key_value_table(flat_key_value_table&& t) noexcept { swap(t); }

  flat_key_value_table& operator=(flat_key_value_table&& t) noexcept {
    swap(t);
    return *this;
  }

  void swap(flat_key_value_table& t) noexcept {
    std::swap(alloc, t.alloc);
    std::swap(size, t.size);
    std::swap(psl, t.psl);
    std::swap(keys, t.keys);
    std::swap(values, t.values);
  }

  void clear() noexcept {
    for (size_type i = 0; i < size; ++i) {
      if (empty(i)) continue;
      destroy(i);
    }
  }

  // private:
  void init() {
    allocate();
    std::fill(psl, psl + size, 0);
  }

  void free() {
    if (empty()) return;
    clear();
    deallocate();
  }

  /// Assumes old stuff has been deallocated.
  void copy(const flat_key_value_table& t) {
    init();
    for (size_type i = 0; i < t.size; ++i) {
      if (!t.valid(i)) continue;
      psl[i] = t.psl[i];
      construct_key(i, t.keys[i]);
    }
  }

  bool empty() const noexcept { return psl == nullptr; }
  bool empty(size_type index) const noexcept { return psl[index] == 0; }
  bool valid(size_type index) const noexcept { return psl[index]; }

  auto entry(size_type index) noexcept {
    return std::pair<const key_type&, value_type&>{keys[index], values[index]};
  }

  auto entry(size_type index) const noexcept {
    return std::pair<const key_type&, const value_type&>{keys[index],
                                                         values[index]};
  }

  void allocate() {
    basic_key_allocator   key_alloc   = alloc;
    basic_value_allocator value_alloc = alloc;
    basic_psl_allocator   psl_alloc   = alloc;

    keys   = key_allocator::allocate(key_alloc, size);
    values = value_allocator::allocate(value_alloc, size);
    psl    = psl_allocator::allocate(psl_alloc, size);
  }

  void deallocate() {
    basic_key_allocator   key_alloc   = alloc;
    basic_value_allocator value_alloc = alloc;
    basic_psl_allocator   psl_alloc   = alloc;

    psl_allocator::deallocate(psl_alloc, psl, size);
    value_allocator::deallocate(value_alloc, values, size);
    key_allocator::deallocate(key_alloc, keys, size);
  }

  template <typename... arguments>
  void construct_key(size_type index, arguments&&... args)  //
      requires std::constructible_from<key_type, arguments...> {
    basic_key_allocator key_alloc = alloc;
    key_allocator::construct(key_alloc, keys + index,
                             std::forward<arguments>(args)...);
  }

  void destroy_key(size_type index) noexcept {
    basic_key_allocator key_alloc = alloc;
    key_allocator::destroy(key_alloc, keys + index);
  }

  template <typename... arguments>
  void construct_value(size_type index, arguments&&... args)  //
      requires std::constructible_from<value_type, arguments...> {
    basic_value_allocator value_alloc = alloc;
    value_allocator::construct(value_alloc, values + index,
                               std::forward<arguments>(args)...);
  }

  void destroy_value(size_type index) noexcept {
    basic_value_allocator value_alloc = alloc;
    value_allocator::destroy(value_alloc, values + index);
  }

  void destroy(size_type index) noexcept {
    destroy_key(index);
    destroy_value(index);
    psl[index] = 0;
  }

  void swap(size_type first, size_type second) {
    using xstd::swap;
    swap(keys[first], keys[second]);
    swap(values[first], values[second]);
  }

  void move(size_type to, size_type from) {
    keys[to]   = std::move(keys[from]);
    values[to] = std::move(values[from]);
  }

  void move_construct(size_type index, size_type from) {
    construct_key(index, std::move(keys[from]));
    construct_value(index, std::move(values[from]));
  }

  void move_construct(size_type index, iterator it) {
    construct_key(index, std::move(it.base->keys[it.index]));
    construct_value(index, std::move(it.base->values[it.index]));
  }

  void move_assign(size_type index, iterator it) {
    keys[index]   = std::move(it.base->keys[it.index]);
    values[index] = std::move(it.base->values[it.index]);
  }

  auto index_iterator(size_type index) { return iterator{this, index}; }

  allocator   alloc  = {};
  size_type   size   = 0;
  psl_type*   psl    = nullptr;
  key_type*   keys   = nullptr;
  value_type* values = nullptr;
};

template <generic::key Key, generic::value Value, generic::allocator Allocator>
inline std::ostream& operator<<(
    std::ostream&                                      os,
    const flat_key_value_table<Key, Value, Allocator>& table) {
  using namespace std;
  os << '\n';
  for (size_t i = 0; i < table.size; ++i) {
    os << setw(15) << i;
    if (!table.psl[i]) {
      os << ' ' << setfill('-') << setw(45) << '\n' << setfill(' ');
      continue;
    }
    os << setw(15) << table.keys[i] << setw(15) << table.values[i] << setw(15)
       << table.psl[i] << '\n';
  }
  return os;
}

}  // namespace lyrahgames::robin_hood::detail
