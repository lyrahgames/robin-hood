#pragma once
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>

namespace lyrahgames::robin_hood::detail {

template <std::destructible Key,
          std::destructible Value,
          typename Allocator = std::allocator<Key>>
struct table {
  // psl_type should be smaller -> uint32_t or even uint16_t
  // psl will not get that long and otherwise
  // it is a bad hash implementation
  using psl_type   = size_t;
  using size_type  = size_t;
  using key_type   = Key;
  using value_type = Value;
  using allocator  = Allocator;
  // Allocator Types
  using basic_key_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<key_type>;
  using key_allocator = std::allocator_traits<basic_key_allocator>;

  using basic_value_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<value_type>;
  using value_allocator = std::allocator_traits<basic_value_allocator>;

  using basic_psl_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<psl_type>;
  using psl_allocator = std::allocator_traits<basic_psl_allocator>;

  template <bool constant>
  struct basic_iterator;

  using iterator       = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;

  table() = default;
  explicit table(size_type s, const allocator& a = {});
  virtual ~table() noexcept;

  table(const table&);
  table& operator=(const table&);

  table(table&&) noexcept;
  table& operator=(table&&) noexcept;

  void swap(table& t) noexcept;

  // Destroys every valid entry in the table.
  void clear() noexcept;

  /// Constructs a new key at given index without bounds checking.
  template <typename... arguments>
  void construct_key(size_type index, arguments&&... args)  //
      requires std::constructible_from<key_type, arguments...>;

  /// Destroys a key at given index without bounds checking.
  void destroy_key(size_type index) noexcept;

  /// Constructs a new value at given index without bounds checking.
  template <typename... arguments>
  void construct_value(size_type index, arguments&&... args)  //
      requires std::constructible_from<value_type, arguments...>;

  /// Destroys a value at given index without bounds checking.
  void destroy_value(size_type index) noexcept;

  /// Destroys key and value at given position and sets the psl invalid.
  void destroy(size_type index) noexcept;

  /// Returns if a given index in the table
  /// has been constructed without bounds checking.
  bool valid(size_type index) const noexcept { return psl[index]; }

  auto begin() noexcept -> iterator;
  auto begin() const noexcept -> const_iterator;
  auto end() noexcept -> iterator;
  auto end() const noexcept -> const_iterator;

  // State
  basic_key_allocator   key_alloc   = {};
  basic_value_allocator value_alloc = {};
  basic_psl_allocator   psl_alloc   = {};

  key_type*   keys   = nullptr;
  value_type* values = nullptr;
  psl_type*   psl    = nullptr;
  size_type   size   = 0;

 private:
  /// Allocates memory for elements.
  void init(size_type s);

  /// Destroys all elements and deallocates memory of elements.
  void free();

  /// Copies all elements contained inside the given map. Assumes current size
  /// is the same as t.size and no element has been inserted.
  void copy(const table& t);
};

}  // namespace lyrahgames::robin_hood::detail

#include <lyrahgames/robin_hood/detail/table.ipp>