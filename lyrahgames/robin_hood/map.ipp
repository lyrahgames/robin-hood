/// @file

namespace lyrahgames::robin_hood {

// Make implementation of members manageable by providing macros
// for template parameters and class namespace.
// Try to mimic the C++ syntax to make parsing possible for other tools.
#define TEMPLATE                                         \
  template <generic::key Key, generic::value Value,      \
            generic::hasher<Key>               Hasher,   \
            generic::equivalence_relation<Key> Equality, \
            generic::allocator                 Allocator>
#define MAP map<Key, Value, Hasher, Equality, Allocator>

TEMPLATE
void MAP::set_max_load_factor(real x) {
  assert((x > 0) || (x < 1));
  max_load_ratio = x;
  max_load       = std::floor(max_load_ratio * table.size);
}

TEMPLATE
inline auto MAP::ideal_index(const key_type& key) const noexcept -> size_type {
  const auto mask = table.size - size_type{1};
  return hash(key) & mask;
}

TEMPLATE
inline auto MAP::next(size_type index) const noexcept -> size_type {
  const auto mask = table.size - size_type{1};
  return (index + size_type{1}) & mask;
}

TEMPLATE
inline auto MAP::basic_lookup_data(const key_type& key) const noexcept
    -> std::tuple<size_type, psl_type, bool> {
  auto index = ideal_index(key);
  auto psl   = psl_type{1};
  for (; psl <= table.psl[index]; ++psl) {
    if (equal(table.keys[index], key)) return {index, psl, true};
    index = next(index);
  }
  return {index, psl, false};
}

TEMPLATE
inline auto MAP::basic_static_insert_data(const key_type& key) const noexcept
    -> std::pair<size_type, psl_type> {
  auto index = ideal_index(key);
  auto psl   = psl_type{1};
  for (; psl <= table.psl[index]; ++psl)
    index = next(index);
  return {index, psl};
}

TEMPLATE
template <generic::forward_reference<Key> K>
void MAP::basic_static_insert(K&& key, size_type index, psl_type psl) {
  // With this, we can use custom swap routines when they are defined as member
  // functions. Otherwise, we try to use the standard.
  using xstd::swap;

  ++load;

  if (!table.psl[index]) {
    table.psl[index] = psl;
    table.construct_key(index, std::forward<K>(key));
    return;
  }

  key_type    tmp_key   = std::move(table.keys[index]);
  mapped_type tmp_value = std::move(table.values[index]);
  table.keys[index]     = std::forward<K>(key);
  std::swap(psl, table.psl[index]);
  ++psl;
  index = next(index);

  for (; table.psl[index]; ++psl) {
    if (psl > table.psl[index]) {
      swap(psl, table.psl[index]);
      swap(tmp_key, table.keys[index]);
      swap(tmp_value, table.values[index]);
    }
    index = next(index);
  }

  table.construct_key(index, std::move(tmp_key));
  table.construct_value(index, std::move(tmp_value));
  table.psl[index] = psl;
}

TEMPLATE
void MAP::set_capacity_and_rehash(size_type c) {
  container old_table{c, alloc};

  table.swap(old_table);
  load     = 0;
  max_load = std::floor(max_load_ratio * table.size);

  for (size_type i = 0; i < old_table.size; ++i) {
    if (!old_table.psl[i]) continue;
    const auto [index, psl] = basic_static_insert_data(old_table.keys[i]);
    basic_static_insert(std::move(old_table.keys[i]), index, psl);
    table.construct_value(index, std::move(old_table.values[i]));
  }
}

TEMPLATE
void MAP::double_capacity_and_rehash() {
  set_capacity_and_rehash(table.size << 1);
}

TEMPLATE
template <generic::forward_reference<Key> K>
auto MAP::basic_insert(K&& key, size_type index, psl_type psl) -> size_type {
  if (overloaded()) {
    double_capacity_and_rehash();
    const auto [i, p] = basic_static_insert_data(key);

    index = i;
    psl   = p;
  }
  basic_static_insert(std::forward<K>(key), index, psl);
  return index;
}

TEMPLATE
template <generic::forwardable<Key> K>
auto MAP::static_insert_key(K&& key) -> size_type {
  // This makes sure key is constructed if it is not a direct forward reference.
  decltype(auto) k         = forward_construct<Key>(std::forward<K>(key));
  auto [index, psl, found] = basic_lookup_data(k);
  if (found)
    throw std::invalid_argument(
        "Failed to insert element that already exists!");
  if (overloaded())
    throw std::overflow_error("Failed to statically insert given element!");
  basic_static_insert(std::forward<decltype(k)>(k), index, psl);
  return index;
}

TEMPLATE
template <generic::forwardable<Key> K>
auto MAP::insert_key(K&& key) -> size_type {
  // This makes sure key is constructed if it is not a direct forward reference.
  decltype(auto) k         = forward_construct<Key>(std::forward<K>(key));
  auto [index, psl, found] = basic_lookup_data(k);
  if (found)
    throw std::invalid_argument(
        "Failed to insert element that already exists!");
  index = basic_insert(std::forward<decltype(k)>(k), index, psl);
  return index;
}

TEMPLATE
template <generic::forwardable<Key> K, generic::forwardable<Value> V>
inline void MAP::static_insert(K&& key, V&& value) {
  const auto index = static_insert_key(std::forward<K>(key));
  table.construct_value(index, std::forward<V>(value));
}

TEMPLATE
template <generic::forwardable<Key> K>
inline void MAP::static_insert(K&& key)  //
    requires std::default_initializable<mapped_type> {
  const auto index = static_insert_key(std::forward<K>(key));
  table.construct_value(index);
}

TEMPLATE
template <generic::forwardable<Key> K, generic::forwardable<Value> V>
inline void MAP::insert(K&& key, V&& value) {
  const auto index = insert_key(std::forward<K>(key));
  table.construct_value(index, std::forward<V>(value));
}

TEMPLATE
template <generic::forwardable<Key> K>
inline void MAP::insert(K&& key)  //
    requires std::default_initializable<mapped_type> {
  const auto index = insert_key(std::forward<K>(key));
  table.construct_value(index);
}

TEMPLATE
template <generic::forwardable<Key> K, typename... arguments>
inline void MAP::static_emplace(K&& key, arguments&&... args)  //
    requires std::constructible_from<mapped_type, arguments...> {
  const auto index = static_insert_key(std::forward<K>(key));
  table.construct_value(index, std::forward<arguments>(args)...);
}

TEMPLATE
template <generic::forwardable<Key> K, typename... arguments>
inline void MAP::emplace(K&& key, arguments&&... args)  //
    requires std::constructible_from<mapped_type, arguments...> {
  const auto index = insert_key(std::forward<K>(key));
  table.construct_value(index, std::forward<arguments>(args)...);
}

TEMPLATE
template <generic::forwardable<Key> K, generic::forwardable<Value> V>
inline bool MAP::try_static_insert(K&& key, V&& value) {
  try {
    static_insert(std::forward<K>(key), std::forward<V>(value));
  } catch (...) {
    return false;
  }
  return true;
}

TEMPLATE
template <generic::forwardable<Key> K>
inline bool MAP::try_static_insert(K&& key)  //
    requires std::default_initializable<mapped_type> {
  try {
    static_insert(std::forward<K>(key));
  } catch (...) {
    return false;
  }
  return true;
}

TEMPLATE
template <generic::forwardable<Key> K, generic::forwardable<Value> V>
void MAP::insert_or_assign(K&& key, V&& value) {
  // This makes sure key is constructed if it is not a direct forward reference.
  decltype(auto) k         = forward_construct<Key>(std::forward<K>(key));
  auto [index, psl, found] = basic_lookup_data(k);
  if (found) {
    table.values[index] = std::forward<V>(value);
    return;
  }
  index = basic_insert(std::forward<decltype(k)>(k), index, psl);
  table.construct_value(index, std::forward<V>(value));
}

TEMPLATE
template <generic::forwardable<Key> K>
auto MAP::operator[](K&& key) -> mapped_type&  //
    requires std::default_initializable<Value> {
  decltype(auto) k         = forward_construct<Key>(std::forward<K>(key));
  auto [index, psl, found] = basic_lookup_data(k);
  if (found) return table.values[index];
  index = basic_insert(std::forward<decltype(k)>(k), index, psl);
  table.construct_value(index);
  return table.values[index];
}

TEMPLATE
auto MAP::operator()(const key_type& key) -> mapped_type& {
  const auto [index, psl, found] = basic_lookup_data(key);
  if (found) return table.values[index];
  throw std::invalid_argument("Failed to find the given key.");
}

TEMPLATE
auto MAP::operator()(const key_type& key) const -> const mapped_type& {
  return const_cast<map&>(*this).operator()(key);
}

TEMPLATE
template <generic::forwardable<Value> V>
void MAP::assign(const key_type& key, V&& value) {
  operator()(key) = std::forward<V>(value);
}

TEMPLATE
auto MAP::lookup_iterator(const key_type& key) noexcept -> iterator {
  const auto [index, psl, found] = basic_lookup_data(key);
  if (found) return {&table, index};
  return end();
}

TEMPLATE
auto MAP::lookup_iterator(const key_type& key) const noexcept
    -> const_iterator {
  const auto [index, psl, found] = basic_lookup_data(key);
  if (found) return {&table, index};
  return end();
}

TEMPLATE
inline bool MAP::contains(const key_type& key) const noexcept {
  const auto [index, psl, found] = basic_lookup_data(key);
  return found;
}

TEMPLATE
void MAP::basic_erase(size_type index) {
  auto next_index = next(index);
  while (table.psl[next_index] > 1) {
    table.keys[index]   = std::move(table.keys[next_index]);
    table.values[index] = std::move(table.values[next_index]);
    table.psl[index]    = table.psl[next_index] - 1;

    index      = next_index;
    next_index = next(next_index);
  }
  table.destroy(index);
  --load;
}

TEMPLATE
bool MAP::erase(const key_type& key) {
  const auto [index, psl, found] = basic_lookup_data(key);
  if (!found) return false;
  basic_erase(index);
  return true;
}

TEMPLATE
void MAP::erase(iterator it) {
  basic_erase(it.index);
}

TEMPLATE
void MAP::erase(const_iterator it) {
  basic_erase(it.index);
}

TEMPLATE
void MAP::reserve(size_type size) {
  if (size <= table.size) return;
  const auto new_size = ceil_pow2(size);
  set_capacity_and_rehash(new_size);
}

TEMPLATE
void MAP::rehash(size_type count) {
  count = std::ceil(count / max_load_ratio);
  if (count <= table.size) return;
  const auto new_size = ceil_pow2(count);
  set_capacity_and_rehash(new_size);
}

TEMPLATE
void MAP::clear() {
  load = 0;
  for (size_type i = 0; i < table.size; ++i)
    if (table.psl[i]) table.destroy(i);
}

TEMPLATE
template <generic::pair_input_iterator<Key, Value> T>
void MAP::insert(T first, T last) {
  for (auto it = first; it != last; ++it) {
    const auto& [key, value] = *it;
    insert_or_assign(key, value);
    // operator[](key) = value;
  }
}

TEMPLATE
template <generic::input_iterator<Key> T, generic::input_iterator<Value> U>
void MAP::insert(T first, T last, U v) {
  for (auto it = first; it != last; ++it, ++v)
    insert_or_assign(*it, *v);
  // operator[](*it) = *v;
}

TEMPLATE
MAP::map(size_type s, const hasher& h, const equality& e, const allocator& a)
    : hash{h}, equal{e}, alloc{a} {
  rehash(s);
}

TEMPLATE
template <generic::pair_input_iterator<Key, Value> T>
MAP::map(T                first,
         T                last,
         size_type        s,
         const hasher&    h,
         const equality&  e,
         const allocator& a)
    : map(std::max(size_type(std::distance(first, last)), s), h, e, a) {
  insert(first, last);
}

TEMPLATE
template <generic::pair_input_iterator<Key, Value> T>
MAP::map(T                first,
         T                last,
         const hasher&    h,
         const equality&  e,
         const allocator& a)
    : map(std::distance(first, last), h, e, a) {
  insert(first, last);
}

// TEMPLATE
// inline std::ostream& operator<<(std::ostream& os, const MAP& m) {
//   using namespace std;
//   os << '\n';
//   for (size_t i = 0; i < m.table.size; ++i) {
//     os << setw(15) << i;
//     if (!m.table.psl[i]) {
//       os << ' ' << setfill('-') << setw(45) << '\n' << setfill(' ');
//       continue;
//     }
//     os << setw(15) << m.table.keys[i] << setw(15) << m.table.values[i]
//        << setw(15) << m.table.psl[i] << '\n';
//   }
//   return os;
// }

#undef TEMPLATE
#undef MAP

}  // namespace lyrahgames::robin_hood