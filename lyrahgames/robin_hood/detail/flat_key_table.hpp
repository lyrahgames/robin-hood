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

namespace lyrahgames::robin_hood::detail {

template <generic::key Key, generic::allocator Allocator = std::allocator<Key>>
struct flat_key_table
    : public basic_iterator_interface<flat_key_table<Key, Allocator>> {
  using size_type = typename traits::size_type;
  using psl_type  = typename traits::psl_type;
  using key_type  = Key;
  using allocator = Allocator;

  using basic_key_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<key_type>;
  using key_allocator = std::allocator_traits<basic_key_allocator>;

  using basic_psl_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<psl_type>;
  using psl_allocator = std::allocator_traits<basic_psl_allocator>;

  using iterator_interface =
      basic_iterator_interface<flat_key_table<Key, Allocator>>;
  using typename iterator_interface::const_iterator;
  using typename iterator_interface::iterator;

  flat_key_table() = default;

  explicit flat_key_table(size_type s, allocator a = {}) : alloc{a}, size{s} {
    init();
  }

  virtual ~flat_key_table() noexcept { free(); }

  flat_key_table(const flat_key_table& t) : alloc{t.alloc}, size{t.size} {
    copy(t);
  }

  flat_key_table& operator=(const flat_key_table& t) {
    free();
    alloc = t.alloc;
    size  = t.size;
    copy(t);
    return *this;
  }

  flat_key_table(flat_key_table&& t) noexcept { swap(t); }

  flat_key_table& operator=(flat_key_table&& t) noexcept {
    swap(t);
    return *this;
  }

  bool empty() const noexcept { return size == 0; }

  bool empty(size_type index) const noexcept { return psls[index] == 0; }

  auto entry(size_type index) noexcept -> const key_type& {
    return keys[index];
  }

  auto entry(size_type index) const noexcept -> const key_type& {
    return keys[index];
  }

  auto index_iterator(size_type index) noexcept {
    return iterator{this, index};
  }

  auto psl(size_type index) noexcept -> psl_type& { return psls[index]; }

  auto psl(size_type index) const noexcept -> const psl_type& {
    return psls[index];
  }

  auto key(size_type index) noexcept -> key_type& { return keys[index]; }

  auto key(size_type index) const noexcept -> const key_type& {
    return keys[index];
  }

  void swap(flat_key_table& t) noexcept {
    std::swap(alloc, t.alloc);
    std::swap(size, t.size);
    std::swap(psls, t.psls);
    std::swap(keys, t.keys);
  }

  void clear() noexcept {
    for (size_type i = 0; i < size; ++i) {
      if (empty(i)) continue;
      destroy(i);
    }
  }

  void copy(const flat_key_table& t) {
    init();
    for (size_type i = 0; i < t.size; ++i) {
      if (t.empty(i)) continue;
      psls[i] = t.psls[i];
      construct_key(i, t.keys[i]);
    }
  }

  void allocate() {
    basic_key_allocator key_alloc = alloc;
    basic_psl_allocator psl_alloc = alloc;

    keys = key_allocator::allocate(key_alloc, size);
    psls = psl_allocator::allocate(psl_alloc, size);
  }

  void deallocate() {
    basic_key_allocator key_alloc = alloc;
    basic_psl_allocator psl_alloc = alloc;

    psl_allocator::deallocate(psl_alloc, psls, size);
    key_allocator::deallocate(key_alloc, keys, size);
  }

  void init() {
    allocate();
    std::fill(psls, psls + size, 0);
  }

  void free() {
    if (empty()) return;
    clear();
    deallocate();
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

  void destroy(size_type index) noexcept {
    destroy_key(index);
    psls[index] = 0;
  }

  void move_construct(size_type index, size_type from) {
    construct_key(index, std::move(keys[from]));
  }

  void move_construct_or_assign(size_type index, psl_type p, iterator it) {
    if (empty(index)) {
      psls[index] = p;
      construct_key(index, std::move(it.base->keys[it.index]));
      return;
    }
    psls[index] = p;
    keys[index] = std::move(it.base->keys[it.index]);
  }

  void move(size_type to, size_type from) { keys[to] = std::move(keys[from]); }

  void swap(size_type first, size_type second) noexcept {
    // With this, we can use custom swap routines when they are defined as
    // member functions. Otherwise, we try to use the standard.
    using xstd::swap;
    swap(keys[first], keys[second]);
  }

  allocator alloc = {};
  size_type size  = 0;
  psl_type* psls  = nullptr;
  key_type* keys  = nullptr;
};

template <generic::key Key, generic::allocator Allocator>
std::ostream& operator<<(std::ostream&                         os,
                         const flat_key_table<Key, Allocator>& table) {
  using namespace std;
  os << '\n';
  for (size_t i = 0; i < table.size; ++i) {
    os << setw(15) << i;
    if (table.empty(i)) {
      os << ' ' << setfill('-') << setw(45) << '\n' << setfill(' ');
      continue;
    }
    os << setw(15) << table.keys[i] << setw(15) << table.psls[i] << '\n';
  }
  return os;
}

}  // namespace lyrahgames::robin_hood::detail
