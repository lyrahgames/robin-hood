#pragma once
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
//
#include <lyrahgames/robin_hood/meta.hpp>
//
#include <lyrahgames/robin_hood/detail/traits.hpp>

namespace lyrahgames::robin_hood::detail {

template <generic::key Key, generic::allocator Allocator = std::allocator<Key>>
struct flat_key_table_base {
  using size_type = typename traits::size_type;
  using psl_type  = typename traits::psl_type;
  using key_type  = Key;
  using allocator = Allocator;

  // Allocator Types
  using basic_key_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<key_type>;
  using key_allocator = std::allocator_traits<basic_key_allocator>;

  using basic_psl_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<psl_type>;
  using psl_allocator = std::allocator_traits<basic_psl_allocator>;

  flat_key_table_base() = default;
  explicit flat_key_table_base(size_type s, allocator a = {})
      : alloc{a}, size{s} {
    init();
  }
  virtual ~flat_key_table_base() noexcept { free(); }

  flat_key_table_base(const flat_key_table_base& t)
      : alloc{t.alloc}, size{t.size} {
    copy(t);
  }
  flat_key_table_base& operator=(const flat_key_table_base& t) {
    free();
    alloc = t.alloc;
    size  = t.size;
    copy(t);
    return *this;
  }

  flat_key_table_base(flat_key_table_base&& t) noexcept { swap(t); }
  flat_key_table_base& operator=(flat_key_table_base&& t) noexcept {
    swap(t);
    return *this;
  }

  void swap(flat_key_table_base& t) noexcept {
    std::swap(alloc, t.alloc);
    std::swap(size, t.size);
    std::swap(psl, t.psl);
    std::swap(keys, t.keys);
  }

  void clear() noexcept {
    for (size_type i = 0; i < size; ++i)
      if (valid(i)) destroy_key(i);
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

  bool valid(size_type index) const noexcept { return psl[index]; }

  // private:
  void init() {
    basic_psl_allocator psl_alloc = alloc;
    basic_key_allocator key_alloc = alloc;

    psl = psl_allocator::allocate(psl_alloc, size);
    std::fill(psl, psl + size, 0);

    keys = key_allocator::allocate(key_alloc, size);
  }

  void free() {
    if (!psl) return;

    clear();

    basic_psl_allocator psl_alloc = alloc;
    basic_key_allocator key_alloc = alloc;

    psl_allocator::deallocate(psl_alloc, psl, size);
    key_allocator::deallocate(key_alloc, keys, size);
  }

  /// Assumes old stuff has been deallocated.
  void copy(const flat_key_table_base& t) {
    init();
    for (size_type i = 0; i < t.size; ++i) {
      if (!t.valid(i)) continue;
      psl[i] = t.psl[i];
      construct_key(i, t.keys[i]);
    }
  }

  // private:
  // State
  allocator alloc = {};
  size_type size  = 0;
  psl_type* psl   = nullptr;
  key_type* keys  = nullptr;
};

}  // namespace lyrahgames::robin_hood::detail

// #include <lyrahgames/robin_hood/detail/flat_key_table_base.ipp>