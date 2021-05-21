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
  using base::basic_lookup_data;

  flat_set()                   = default;
  virtual ~flat_set() noexcept = default;

  flat_set(const flat_set&) = default;
  flat_set& operator=(const flat_set&) = default;

  flat_set(flat_set&&) noexcept = default;
  flat_set& operator=(flat_set&&) noexcept = default;

  explicit flat_set(size_type        s,
                    const hasher&    h = {},
                    const equality&  e = {},
                    const allocator& a = {}) {
    reserve_capacity(s);
    base::hash  = h;
    base::equal = e;
  }

  bool empty() const noexcept { return base::empty(); }

  auto size() const noexcept { return base::size(); }

  auto capacity() const noexcept { return base::capacity(); }

  auto load_factor() const noexcept { return base::load_factor(); }

  auto max_load_factor() const noexcept { return base::max_load_factor(); }

  void set_max_load_factor(real x) { base::set_max_load_factor(x); }

  auto begin() noexcept -> iterator { return base::table.begin(); }

  auto begin() const noexcept -> const_iterator { return base::table.begin(); }

  auto end() noexcept -> iterator { return base::table.end(); }

  auto end() const noexcept -> const_iterator { return base::table.end(); }

  const auto& data() const noexcept { return base::table; }

  template <generic::forwardable<key_type> K>
  void static_insert(K&& key) {
    base::static_insert_key(std::forward<K>(key));
  }

  template <generic::forwardable<key_type> K>
  void insert(K&& key) {
    base::insert_key(std::forward<K>(key));
  }

  bool contains(const key_type& key) const noexcept {
    return base::contains(key);
  }

  void remove(const key_type& key) { base::remove(key); }

  void remove(iterator it) { base::remove(it); }

  void remove(const_iterator it) { base::remove(it); }

  void reserve(size_type count) { base::reserve(count); }

  void reserve_capacity(size_type count) { base::reserve_capacity(count); }

  auto lookup_iterator(const key_type& key) noexcept -> iterator {
    return base::lookup_iterator(key);
  }

  auto lookup_iterator(const key_type& key) const noexcept -> const_iterator {
    return base::lookup_iterator(key);
  }

  auto operator[](const key_type& key) -> flat_set& {
    insert(key);
    return *this;
  }

  bool operator()(const key_type& key) const noexcept {
    return base::contains(key);
  }
};

TEMPLATE
std::ostream& operator<<(std::ostream& os, const FLAT_SET& s) {
  using namespace std;
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

#undef FLAT_SET
#undef TEMPLATE

}  // namespace lyrahgames::robin_hood
