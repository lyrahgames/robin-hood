#pragma once
#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
//
#include <lyrahgames/xstd/math.hpp>
#include <lyrahgames/xstd/swap.hpp>
//
#include <lyrahgames/robin_hood/meta.hpp>
//
#include <lyrahgames/robin_hood/detail/flat_key_table.hpp>
#include <lyrahgames/robin_hood/detail/hash_base.hpp>

namespace lyrahgames::robin_hood {

template <generic::key                       Key,
          generic::hasher<Key>               Hasher    = std::hash<Key>,
          generic::equivalence_relation<Key> Equality  = std::equal_to<Key>,
          generic::allocator                 Allocator = std::allocator<Key>>
class flat_set;

#define TEMPLATE                                           \
  template <generic::key Key, generic::hasher<Key> Hasher, \
            generic::equivalence_relation<Key> Equality,   \
            generic::allocator                 Allocator>
#define FLAT_SET flat_set<Key, Hasher, Equality, Allocator>

TEMPLATE
using flat_set_base =
    detail::hash_base<detail::flat_key_table<Key, Allocator>, Hasher, Equality>;

TEMPLATE
class flat_set : private flat_set_base<Key, Hasher, Equality, Allocator> {
 public:
  using base           = flat_set_base<Key, Hasher, Equality, Allocator>;
  using key_type       = Key;
  using allocator      = Allocator;
  using hasher         = Hasher;
  using equality       = Equality;
  using size_type      = typename base::size_type;
  using real           = typename base::real;
  using const_iterator = typename base::const_iterator;
  using iterator       = typename base::iterator;

  /// Used for statistics and logging tests.
  using base::lookup_data;

  flat_set()                   = default;
  virtual ~flat_set() noexcept = default;

  flat_set(const flat_set&) = default;
  flat_set& operator=(const flat_set&) = default;

  flat_set(flat_set&&) noexcept = default;
  flat_set& operator=(flat_set&&) noexcept = default;

  explicit flat_set(size_type s,
                    hasher    h = {},
                    equality  e = {},
                    allocator a = {})
      : base(s, h, e, a) {}

  template <generic::input_range<key_type> T>
  explicit flat_set(const T& data) : base(0) {
    insert(data);
  }

  template <generic::input_range<key_type> T>
  explicit flat_set(const T& data, hasher h, equality e = {}, allocator a = {})
      : flat_set(0, h, e, a) {
    insert(data);
  }

  /// Checks if the set contains zero elements.
  bool empty() const noexcept { return base::empty(); }

  /// Returns the count of inserted elements.
  auto size() const noexcept { return base::size(); }

  /// Returns the maximum number of storable elements in the current table.
  /// The capacity is doubled when the the load factor exceeds
  /// the maximum load factor.
  auto capacity() const noexcept { return base::capacity(); }

  /// Returns the current load factor of the set.
  /// The load factor is the quotient of size and capacity.
  auto load_factor() const noexcept { return base::load_factor(); }

  /// Returns the maximum load factor the set is allowed to have before
  /// rehashing all elements with a bigger capacity.
  auto max_load_factor() const noexcept { return base::max_load_factor(); }

  /// Sets the maximum load factor the set is allowed to have before
  /// rehashing all elements with a bigger capacity.
  /// Setting the maximum load factor to smaller values, may trigger a
  /// reallocation and rehashing of all contained keys.
  void set_max_load_factor(real x) { base::set_max_load_factor(x); }

  /// Return an iterator to the beginning of the set.
  auto begin() noexcept -> iterator { return base::table.begin(); }

  /// Return a constant iterator to the beginning of the set.
  auto begin() const noexcept -> const_iterator { return base::table.begin(); }

  /// Return an iterator to the end of the set.
  auto end() noexcept -> iterator { return base::table.end(); }

  /// Return a constant iterator to the end of the set.
  auto end() const noexcept -> const_iterator { return base::table.end(); }

  /// Returns a constant reference to the underlying table.
  /// Mainly used for debugging and logging.
  const auto& data() const noexcept { return base::table; }

  /// Checks if an element has already been inserted into the map.
  bool contains(const key_type& key) const noexcept {
    return base::contains(key);
  }

  /// Creates an iterator pointing to the given key.
  /// If this is not possible, returns the end iterator.
  auto lookup(const key_type& key) noexcept -> iterator {
    return base::lookup(key);
  }

  /// Creates a constant iterator pointing to the given key.
  /// If this is not possible, return the end iterator. @see lookup
  auto lookup(const key_type& key) const noexcept -> const_iterator {
    return base::lookup(key);
  }

  /// Checks if an element has already been inserted into the map.
  bool operator()(const key_type& key) const noexcept {
    return base::contains(key);
  }

  /// Reserves enough memory in the underlying table by creating a new temporary
  /// table with the given size ceiled to the next power of two, rehashing all
  /// elements into it, and swapping its content with the actual table.
  /// In this case, all iterators and pointers become invalid.
  /// If the given size is smaller than the current table size, nothing happens.
  void reserve_capacity(size_type count) { base::reserve_capacity(count); }

  /// Reserves enough memory in the underlying table such that 'count' elements
  /// could be inserted without implicitly triggering a rehash with respect to
  /// the current maximum allowed load factor. @see reserve_capacity
  void reserve(size_type count) { base::reserve(count); }

  /// Clears all the contents of the set without changing its capacitcy.
  void clear() { base::clear(); }

  /// Statically inserts a given element into the set without reallocation and
  /// rehashing. If a reallocation would take place, the functions throws an
  /// exception 'std::overlow_error'. If the key has already been inserted, the
  /// function throws an exception of type 'std::invalid_argument'.
  template <generic::forwardable<key_type> K>
  void static_insert(K&& key) {
    base::static_insert_key(std::forward<K>(key));
  }

  /// Statically inserts a given element into the set without reallocation and
  /// rehashing. If a reallocation would take place or if the key has already
  /// been inserted, the function does nothing.
  template <generic::forwardable<key_type> K>
  void try_static_insert(K&& key) {
    base::try_static_insert_key(std::forward<K>(key));
  }

  /// Inserts a given element into the set with possible reallocation and
  /// rehashing. If the element has already been inserted, the
  /// function throws an exception of type 'std::invalid_argument'.
  template <generic::forwardable<key_type> K>
  void insert(K&& key) {
    base::insert_key(std::forward<K>(key));
  }

  /// Inserts a given element into the set with possible reallocation and
  /// rehashing. If the key has already been inserted, nothing is done.
  template <generic::forwardable<key_type> K>
  void try_insert(K&& key) {
    base::try_insert_key(std::forward<K>(key));
  }

  /// Inserts elements into the set by using the given input range.
  template <generic::input_range<key_type> T>
  void insert(const T& data) {
    reserve(std::ranges::size(data) + size());
    for (const auto& k : data)
      try_static_insert(k);
  }

  /// Inserts the given key into the set and returns a reference to the set
  /// itself. This function can be chained. No exception is thrown if the given
  /// element already exists.
  template <generic::forwardable<key_type> K>
  auto operator[](K&& key) -> flat_set& {
    try_insert(std::forward<K>(key));
    return *this;
  }

  /// Removes the given element from the set. If there is no such element,
  /// throws an exception of type 'std::invalid_argument'.
  void remove(const key_type& key) { base::remove(key); }

  /// Removes the element pointed to by the given iterator.
  /// This functions assumes the iterator is pointing to an existing element.
  void remove(iterator it) { base::remove(it); }

  /// Removes the element pointed to by the given iterator.
  /// This functions assumes the iterator is pointing to an existing element.
  void remove(const_iterator it) { base::remove(it); }
};

TEMPLATE
std::ostream& operator<<(std::ostream& os, const FLAT_SET& s) {
  using namespace std;
  if (s.empty()) return os << "{}";
  auto it = s.begin();
  os << "{ " << *it++;
  for (; it != s.end(); ++it)
    os << ", " << *it;
  return os << " }";
}

template <generic::key                       Key,
          generic::hasher<Key>               Hasher    = std::hash<Key>,
          generic::equivalence_relation<Key> Equality  = std::equal_to<Key>,
          generic::allocator                 Allocator = std::allocator<Key>>
inline auto auto_flat_set(size_t           size,
                          const Hasher&    hash  = {},
                          const Equality&  equal = {},
                          const Allocator& alloc = {}) {
  return FLAT_SET(size, hash, equal, alloc);
}

template <std::ranges::input_range                       T,
          generic::hasher<std::ranges::range_value_t<T>> Hasher =
              std::hash<std::ranges::range_value_t<T>>,
          generic::equivalence_relation<std::ranges::range_value_t<T>>
              Equality = std::equal_to<std::ranges::range_value_t<T>>,
          generic::allocator Allocator =
              std::allocator<std::ranges::range_value_t<T>>>
inline auto auto_flat_set(const T&         list,
                          const Hasher&    hash  = {},
                          const Equality&  equal = {},
                          const Allocator& alloc = {}) {
  return flat_set<std::ranges::range_value_t<T>, Hasher, Equality, Allocator>(
      list, hash, equal, alloc);
}

#undef FLAT_SET
#undef TEMPLATE

}  // namespace lyrahgames::robin_hood
