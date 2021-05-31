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

  /// Checks if the map contains zero elements.
  bool empty() const noexcept { return base::empty(); }

  /// Returns the count of inserted elements.
  auto size() const noexcept { return base::size(); }

  /// Returns the maximum number of storable elements in the current table.
  /// The capacity is doubled when the the load factor exceeds
  /// the maximum load factor.
  auto capacity() const noexcept { return base::capacity(); }

  /// Returns the current load factor of the map.
  /// The load factor is the quotient of size and capacity.
  auto load_factor() const noexcept { return base::load_factor(); }

  /// Returns the maximum load factor the map is allowed to have before
  /// rehashing all elements with a bigger capacity.
  auto max_load_factor() const noexcept { return base::max_load_factor(); }

  /// Sets the maximum load factor the map is allowed to have before
  /// rehashing all elements with a bigger capacity.
  /// Setting the maximum load factor to smaller values, may trigger a
  /// reallocation and rehashing of all contained values.
  void set_max_load_factor(real x) { base::set_max_load_factor(x); }

  /// Return an iterator to the beginning of the map.
  auto begin() noexcept -> iterator { return base::table.begin(); }

  /// Return a constant iterator to the beginning of the map.
  auto begin() const noexcept -> const_iterator { return base::table.begin(); }

  /// Return an iterator to the end of the map.
  auto end() noexcept -> iterator { return base::table.end(); }

  /// Return a constant iterator to the end of the map.
  auto end() const noexcept -> const_iterator { return base::table.end(); }

  /// Returns a constant reference to the underlying table.
  /// Mainly used for debugging and logging.
  const auto& data() const noexcept { return base::table; }

  /// Checks if an element with given key has already
  /// been inserted into the map.
  bool contains(const key_type& key) const noexcept {
    return base::contains(key);
  }

  /// Creates an iterator pointing to an element with the given key.
  /// If this is not possible, returns the end iterator.
  auto lookup(const key_type& key) noexcept -> iterator {
    return base::lookup(key);
  }

  /// Creates a constant iterator pointing to an element with the given key.
  /// If this is not possible, return the end iterator. @see lookup
  auto lookup(const key_type& key) const noexcept -> const_iterator {
    return base::lookup(key);
  }

  /// Returns a reference to the mapped value of the given key. If no such
  /// element exists, an exception of type std::invalid_argument is thrown.
  auto operator()(const key_type& key) -> mapped_type& {
    const auto [index, psl, found] = base::lookup_data(key);
    if (found) return base::table.value(index);
    throw std::invalid_argument("Failed to find the given key.");
  }

  /// Returns a constant reference to the mapped value of the given key. If no
  /// such element exists, an exception of type std::invalid_argument is thrown.
  auto operator()(const key_type& key) const -> const mapped_type& {
    return const_cast<flat_map&>(*this).operator()(key);
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

  /// Clears all the contents of the map without changing its capacitcy.
  void clear() { base::clear(); }

  /// Statically insert a given element into the map without reallocation and
  /// rehashing. If a reallocation would take place, the functions throws an
  /// exception 'std::overlow_error'. If the key has already been inserted, the
  /// function throws an exception of type 'std::invalid_argument'.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void static_insert(K&& key, V&& value) {
    const auto index = base::static_insert_key(std::forward<K>(key));
    base::table.construct_value(index, std::forward<V>(value));
  }

  /// Statically insert an element based on the given key with a default
  /// constructed value into the map without reallocation and rehashing. If a
  /// reallocation would take place, the functions throws an exception
  /// 'std::overlow_error'. If the key has already been inserted, the function
  /// throws an exception of type 'std::invalid_argument'.
  template <generic::forwardable<key_type> K>
  void static_insert(K&& key)  //
      requires std::default_initializable<mapped_type> {
    const auto index = base::static_insert_key(std::forward<K>(key));
    base::table.construct_value(index);
  }

  /// Statically insert a given element into the map without reallocation and
  /// rehashing. If a reallocation would take place or if the key has already
  /// been inserted, the function does nothing.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void try_static_insert(K&& key, V&& value) {
    const auto [index, done] =
        base::try_static_insert_key(std::forward<K>(key));
    if (!done) return;
    base::table.construct_value(index, std::forward<V>(value));
  }

  /// Statically insert a given element into the map without reallocation and
  /// rehashing. The value of the element is default constructed. If a
  /// reallocation would take place or if the key has already been inserted, the
  /// function does nothing.
  template <generic::forwardable<key_type> K>
  void try_static_insert(K&& key)  //
      requires std::default_initializable<mapped_type> {
    const auto [index, done] =
        base::try_static_insert_key(std::forward<K>(key));
    if (!done) return;
    base::table.construct_value(index);
  }

  /// Statically inserts a given element into the map without reallocation and
  /// rehashing and without checking for overload. The function assumes that
  /// after insertion the maximum load factor will not be exceeded. If the key
  /// has already been inserted, the function does nothing.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void nocheck_static_insert(K&& key, V&& value) {
    const auto [index, done] =
        base::nocheck_static_insert_key(std::forward<K>(key));
    if (!done) return;
    base::table.construct_value(index, std::forward<V>(value));
  }

  /// Statically insert a given element into the map without reallocation and
  /// rehashing and without checking for overload. The function assumes that
  /// after insertion the maximum load factor will not be exceeded. The value of
  /// the element is default constructed. If the key has already been inserted,
  /// the function does nothing.
  template <generic::forwardable<key_type> K>
  void nocheck_static_insert(K&& key)  //
      requires std::default_initializable<mapped_type> {
    const auto [index, done] =
        base::nocheck_static_insert_key(std::forward<K>(key));
    if (!done) return;
    base::table.construct_value(index);
  }

  /// Insert a given element into the map with possible reallocation and
  /// rehashing. If the key has already been inserted, the
  /// function throws an exception of type 'std::invalid_argument'.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void insert(K&& key, V&& value) {
    const auto index = base::insert_key(std::forward<K>(key));
    base::table.construct_value(index, std::forward<V>(value));
  }

  /// Insert a given element into the map with possible reallocation and
  /// rehashing. The value is default constructed. If the key has already been
  /// inserted, an exception of type 'std::invalid_argument' is thrown.
  template <generic::forwardable<key_type> K>
  void insert(K&& key)  //
      requires std::default_initializable<mapped_type> {
    const auto index = base::insert_key(std::forward<K>(key));
    base::table.construct_value(index);
  }

  /// Insert a given element into the map with possible reallocation and
  /// rehashing. If the key has already been inserted, nothing is done.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void try_insert(K&& key, V&& value) {
    const auto [index, done] = base::try_insert_key(std::forward<K>(key));
    if (!done) return;
    base::table.construct_value(index, std::forward<V>(value));
  }

  /// Insert a given element into the map with possible reallocation and
  /// rehashing. The value is default constructed. If the key has already been
  /// inserted, nothing is done.
  template <generic::forwardable<key_type> K>
  void try_insert(K&& key)  //
      requires std::default_initializable<mapped_type> {
    const auto [index, done] = base::try_insert_key(std::forward<K>(key));
    if (!done) return;
    base::table.construct_value(index);
  }

  /// Insert pair of elements into the map by using the given input range.
  /// If a key occurs multiple times then the last pair will be used to set the
  /// value of the respective element.
  template <generic::pair_input_range<key_type, mapped_type> T>
  void insert(const T& data) {
    reserve(std::ranges::size(data) + size());
    for (const auto& [k, v] : data)
      nocheck_static_insert_or_assign(k, v);
  }

  /// Inserts range of elements into the map by providing keys and values inside
  /// separate input ranges. If a key occurs multiple times then the last
  /// occurence is used to set its value.
  template <generic::input_range<key_type>    K,
            generic::input_range<mapped_type> V>
  void insert(const K& keys, const V& values) {
    using namespace std;
    assert(ranges::size(keys) == ranges::size(values));
    reserve(ranges::size(keys) + size());
    auto v = ranges::begin(values);
    for (auto k = ranges::begin(keys); k != ranges::end(keys); ++k, ++v)
      nocheck_static_insert_or_assign(*k, *v);
  }

  /// Statically emplace a new element into the map by constructing its value in
  /// place. This function uses perfect forwarding construction.
  template <generic::forwardable<key_type> K, typename... arguments>
  void static_emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...> {
    const auto index = base::static_insert_key(std::forward<K>(key));
    base::table.construct_value(index, std::forward<arguments>(args)...);
  }

  /// Statically emplaces a new element into the map by constructing its value
  /// in place. This function uses perfect forwarding construction. If the given
  /// key already exists or if the map would have to reallocate new storage, the
  /// function does nothing.
  template <generic::forwardable<key_type> K, typename... arguments>
  void try_static_emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...> {
    const auto [index, done] =
        base::try_static_insert_key(std::forward<K>(key));
    if (!done) return;
    base::table.construct_value(index, std::forward<arguments>(args)...);
  }

  /// Statically emplaces a new element into the map by constructing its value
  /// in place. This function uses perfect forwarding construction. If the given
  /// key already exists, the function does nothing. Furthermore, the function
  /// assumes that after emplacement the maximum load factor is not exceeded.
  template <generic::forwardable<key_type> K, typename... arguments>
  void nocheck_static_emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...> {
    const auto [index, done] =
        base::nocheck_static_insert_key(std::forward<K>(key));
    if (!done) return;
    base::table.construct_value(index, std::forward<arguments>(args)...);
  }

  /// Emplace a new element into the map by constructing its value in place.
  /// This function uses perfect forwarding construction.
  template <generic::forwardable<key_type> K, typename... arguments>
  void emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...> {
    const auto index = base::insert_key(std::forward<K>(key));
    base::table.construct_value(index, std::forward<arguments>(args)...);
  }

  /// Emplace a new element into the map by constructing its value in
  /// place. This function uses perfect forwarding construction.
  template <generic::forwardable<key_type> K, typename... arguments>
  void try_emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...> {
    const auto index = base::try_insert_key(std::forward<K>(key));
    base::table.construct_value(index, std::forward<arguments>(args)...);
  }

  /// Access the element given by key and assign the given value to it.
  /// If the key does not exist then an exception of type
  /// 'std::invalid_argument' is thrown.
  template <generic::forwardable<mapped_type> V>
  void assign(const key_type& key, V&& value) {
    operator()(key) = std::forward<V>(value);
  }

  /// Statically inserts an element if it not already exists.
  /// Otherwise, assigns a new value to it.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void nocheck_static_insert_or_assign(K&& key, V&& value) {
    decltype(auto) k         = forward_construct<Key>(std::forward<K>(key));
    auto [index, psl, found] = base::lookup_data(k);
    if (found) {
      base::table.value(index) = std::forward<V>(value);
      return;
    }
    base::basic_static_insert_key(index, psl, std::forward<decltype(k)>(k));
    base::table.construct_value(index, std::forward<V>(value));
  }

  /// Inserts an element if it not already exists.
  /// Otherwise, assigns a new value to it.
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

  /// Insert or access the element given by the key. If the key has already been
  /// inserted, the functions returns a reference to its value. Otherwise, the
  /// key will be inserted with a default initialized value to which a reference
  /// is returned.
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

  /// Removes an element from the map with the given key.
  /// If there is no such element, throws an exception of type
  /// 'std::invalid_argument'.
  void remove(const key_type& key) { base::remove(key); }

  /// Removes an element from the map with the given key.
  /// If there is no such element, nothing is done.
  void try_remove(const key_type& key) { base::try_remove(key); }

  /// Removes the element pointed to by the given iterator.
  /// This functions assumes the iterator is pointing to an existing element.
  void remove(iterator it) { base::remove(it); }

  /// Removes the element pointed to by the given iterator.
  /// This functions assumes the iterator is pointing to an existing element.
  void remove(const_iterator it) { base::remove(it); }
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