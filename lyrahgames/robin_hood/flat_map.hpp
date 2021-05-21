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
  using base::basic_lookup_data;

  flat_map()                   = default;
  virtual ~flat_map() noexcept = default;

  flat_map(const flat_map&) = default;
  flat_map& operator=(const flat_map&) = default;

  flat_map(flat_map&&) noexcept = default;
  flat_map& operator=(flat_map&&) noexcept = default;

  explicit flat_map(size_type        s,
                    const hasher&    h = {},
                    const equality&  e = {},
                    const allocator& a = {}) {
    reserve_capacity(s);
    base::hash  = h;
    base::equal = e;
  }

  template <generic::pair_input_range<key_type, mapped_type> T>
  explicit flat_map(const T& data) {
    insert(data);
  }

  explicit flat_map(
      std::initializer_list<std::pair<key_type, mapped_type>> list) {
    reserve(list.size());
    for (const auto& [k, v] : list)
      static_insert(k, v);
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

  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void insert(K&& key, V&& value) {
    const auto index = base::insert_key(std::forward<K>(key));
    base::table.construct_value(index, std::forward<V>(value));
  }

  template <generic::pair_input_range<key_type, mapped_type> T>
  void static_insert(const T& data) {
    reserve(std::ranges::size(data));
    for (const auto& [k, v] : data)
      static_insert(k, v);
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

  template <generic::forwardable<key_type> K>
  auto operator[](K&& key) -> mapped_type&  //
      requires std::default_initializable<mapped_type> {
    decltype(auto) k = forward_construct<key_type>(std::forward<K>(key));
    auto [index, psl, found] = basic_lookup_data(k);
    if (found) return base::table.values[index];
    index = base::basic_insert(std::forward<decltype(k)>(k), index, psl);
    base::table.construct_value(index);
    return base::table.values[index];
  }

  auto operator()(const key_type& key) -> mapped_type& {
    const auto [index, psl, found] = base::basic_lookup_data(key);
    if (found) return base::table.values[index];
    throw std::invalid_argument("Failed to find the given key.");
  }

  auto operator()(const key_type& key) const -> const mapped_type& {
    return const_cast<flat_map&>(*this).operator()(key);
  }
};

TEMPLATE
inline std::ostream& operator<<(std::ostream& os, const FLAT_MAP& m) {
  using namespace std;
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