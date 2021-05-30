#pragma once
#include <lyrahgames/robin_hood/meta.hpp>
//
#include <lyrahgames/robin_hood/detail/flat_key_value_table.hpp>
#include <lyrahgames/robin_hood/detail/hash_base.hpp>

namespace lyrahgames::robin_hood {

template <generic::key                       Key,
          generic::value                     Value,
          generic::hasher<Key>               Hasher    = std::hash<Key>,
          generic::equivalence_relation<Key> Equality  = std::equal_to<Key>,
          generic::allocator                 Allocator = std::allocator<Key>>
class flat_map;

#define TEMPLATE                                         \
  template <generic::key Key, generic::value Value,      \
            generic::hasher<Key>               Hasher,   \
            generic::equivalence_relation<Key> Equality, \
            generic::allocator                 Allocator>
#define FLAT_MAP flat_map<Key, Value, Hasher, Equality, Allocator>

TEMPLATE
using flat_map_base =
    detail::hash_base<detail::flat_key_value_table<Key, Value, Allocator>,
                      Hasher,
                      Equality>;

TEMPLATE
class flat_map
    : private flat_map_base<Key, Value, Hasher, Equality, Allocator> {
 public:
  using base           = flat_map_base<Key, Value, Hasher, Equality, Allocator>;
  using key_type       = Key;
  using mapped_type    = Value;
  using allocator      = Allocator;
  using hasher         = Hasher;
  using equality       = Equality;
  using size_type      = typename base::size_type;
  using real           = typename base::real;
  using const_iterator = typename base::const_iterator;
  using iterator       = typename base::iterator;

  /// Used for statistics and logging tests.
  using base::lookup_data;

  flat_map() = default;

  explicit flat_map(size_type        s,
                    const hasher&    h = {},
                    const equality&  e = {},
                    const allocator& a = {})
      : base(s, h, e, a) {}

  flat_map(size_type s, const allocator& a)
      : flat_map(s, hasher{}, equality{}, a) {}

  flat_map(size_type s, const hasher& h, const allocator& a)
      : flat_map(s, h, equality{}, a) {}

  template <generic::pair_input_range<key_type, mapped_type> T>
  explicit flat_map(const T&         data,
                    const hasher&    h = {},
                    const equality&  e = {},
                    const allocator& a = {})
      : flat_map(0, h, e, a) {
    insert(data);
  }

  explicit flat_map(
      std::initializer_list<std::pair<key_type, mapped_type>> list) {
    reserve(list.size());
    for (const auto& [k, v] : list) {
      try {
        static_insert(k, v);
      } catch (std::invalid_argument&) {
        operator()(k) = v;
      }
    }
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

  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void static_insert(K&& key, V&& value) {
    const auto index = base::static_insert_key(std::forward<K>(key));
    base::table.construct_value(index, std::forward<V>(value));
  }

  template <generic::forwardable<key_type> K>
  void static_insert(K&& key)  //
      requires std::default_initializable<mapped_type> {
    const auto index = base::static_insert_key(std::forward<K>(key));
    base::table.construct_value(index);
  }

  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void insert(K&& key, V&& value) {
    const auto index = base::insert_key(std::forward<K>(key));
    base::table.construct_value(index, std::forward<V>(value));
  }

  template <generic::pair_input_range<key_type, mapped_type> T>
  void insert(const T& data) {
    reserve(std::ranges::size(data) + size());
    for (const auto& [k, v] : data) {
      try {
        static_insert(k, v);
      } catch (std::invalid_argument&) {
      }
    }
  }

  template <generic::input_range<key_type>    K,
            generic::input_range<mapped_type> V>
  void insert(const K& keys, const V& values) {
    using namespace std;
    assert(ranges::size(keys) == ranges::size(values));
    reserve(ranges::size(keys) + size());
    auto v = ranges::begin(values);
    for (auto k = ranges::begin(keys); k != ranges::end(keys); ++k, ++v) {
      try {
        static_insert(*k, *v);
      } catch (std::invalid_argument&) {
      }
    }
  }

  template <generic::forwardable<mapped_type> V>
  void assign(const key_type& key, V&& value) {
    operator()(key) = std::forward<V>(value);
  }

  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void insert_or_assign(K&& key, V&& value) {
    decltype(auto) k         = forward_construct<Key>(std::forward<K>(key));
    auto [index, psl, found] = base::lookup_data(k);
    if (found) {
      base::table.value(index) = std::forward<V>(value);
      return;
    }
    index = base::basic_insert_key(index, psl, std::forward<decltype(k)>(k));
    base::table.construct_value(index, std::forward<V>(value));
  }

  bool contains(const key_type& key) const noexcept {
    return base::contains(key);
  }

  void remove(const key_type& key) { base::remove(key); }

  void remove(iterator it) { base::remove(it); }

  void remove(const_iterator it) { base::remove(it); }

  void reserve(size_type count) { base::reserve(count); }

  void reserve_capacity(size_type count) { base::reserve_capacity(count); }

  void clear() { base::clear(); }

  auto lookup(const key_type& key) noexcept -> iterator {
    return base::lookup(key);
  }

  auto lookup(const key_type& key) const noexcept -> const_iterator {
    return base::lookup(key);
  }

  template <generic::forwardable<key_type> K>
  auto operator[](K&& key) -> mapped_type&  //
      requires std::default_initializable<mapped_type> {
    decltype(auto) k = forward_construct<key_type>(std::forward<K>(key));
    auto [index, psl, found] = base::lookup_data(k);
    if (found) return base::table.value(index);
    index = base::basic_insert_key(index, psl, std::forward<decltype(k)>(k));
    base::table.construct_value(index);
    return base::table.value(index);
  }

  auto operator()(const key_type& key) -> mapped_type& {
    const auto [index, psl, found] = base::lookup_data(key);
    if (found) return base::table.value(index);
    throw std::invalid_argument("Failed to find the given key.");
  }

  auto operator()(const key_type& key) const -> const mapped_type& {
    return const_cast<flat_map&>(*this).operator()(key);
  }
};

template <generic::key                       Key,
          generic::value                     Value,
          generic::hasher<Key>               Hasher    = std::hash<Key>,
          generic::equivalence_relation<Key> Equality  = std::equal_to<Key>,
          generic::allocator                 Allocator = std::allocator<Key>>
inline auto auto_flat_map(size_t           size,
                          const Hasher&    hash  = {},
                          const Equality&  equal = {},
                          const Allocator& alloc = {}) {
  return flat_map<Key, Value, Hasher, Equality, Allocator>(size, hash, equal,
                                                           alloc);
}

template <generic::key                       Key,
          generic::value                     Value,
          generic::hasher<Key>               Hasher    = std::hash<Key>,
          generic::equivalence_relation<Key> Equality  = std::equal_to<Key>,
          generic::allocator                 Allocator = std::allocator<Key>>
inline auto auto_flat_map(std::initializer_list<std::pair<Key, Value>> list,
                          const Hasher&    hash  = {},
                          const Equality&  equal = {},
                          const Allocator& alloc = {}) {
  return flat_map<Key, Value, Hasher, Equality, Allocator>(list, hash, equal,
                                                           alloc);
}

TEMPLATE
inline std::ostream& operator<<(std::ostream& os, const FLAT_MAP& m) {
  using namespace std;
  if (m.empty()) return os << "{}";
  auto it            = m.begin();
  const auto& [k, v] = *it;
  os << "{ "
     << "(" << k << " -> " << v << ")";
  ++it;
  for (; it != m.end(); ++it) {
    const auto& [k, v] = *it;
    os << ", "
       << "(" << k << " -> " << v << ")";
  }
  return os << " }";
}

#undef FLAT_MAP
#undef TEMPLATE

}  // namespace lyrahgames::robin_hood