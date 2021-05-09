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
  max_load = x;
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
inline auto MAP::lookup_data(const key_type& key) const noexcept
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
inline auto MAP::static_insert_data(const key_type& key) const noexcept
    -> std::pair<size_type, psl_type> {
  auto index = ideal_index(key);
  auto psl   = psl_type{1};
  for (; psl <= table.psl[index]; ++psl)
    index = next(index);
  return {index, psl};
}

TEMPLATE
template <generic::forward_reference<Key> K>
void MAP::static_insert(K&& key, size_type index, psl_type psl) {
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
      std::swap(psl, table.psl[index]);
      std::swap(tmp_key, table.keys[index]);
      std::swap(tmp_value, table.values[index]);
    }
    index = next(index);
  }

  table.construct_key(index, std::move(tmp_key));
  table.construct_value(index, std::move(tmp_value));
  table.psl[index] = psl;
}

TEMPLATE
void MAP::set_capacity_and_rehash(size_type c) {
  container old_table{c};
  table.swap(old_table);

  for (size_type i = 0; i < old_table.size; ++i) {
    if (!old_table.psl[i]) continue;
    const auto [index, psl] = static_insert_data(old_table.keys[i]);
    static_insert(old_table.keys[i], index, psl);
    table.construct_value(index, std::move(old_table.values[i]));
  }
}

TEMPLATE
void MAP::double_capacity_and_rehash() {
  set_capacity_and_rehash(table.size << 1);
}

TEMPLATE
auto MAP::insert(const key_type& key, size_type index, psl_type psl)
    -> size_type {
  ++load;
  if (overloaded()) {
    double_capacity_and_rehash();
    const auto [i, p] = static_insert_data(key);
    index             = i;
    psl               = p;
  }
  static_insert(key, index, psl);
  return index;
}

TEMPLATE
bool MAP::insert(const key_type& key, const mapped_type& value) {
  auto [index, psl, found] = lookup_data(key);
  if (found) return false;
  index = insert(key, index, psl);
  table.construct_value(index, value);
  return true;
}

TEMPLATE
bool MAP::insert_or_assign(const key_type& key, const mapped_type& value) {
  auto [index, psl, found] = lookup_data(key);
  if (found) {
    table.values[index] = value;
    return false;
  }
  index = insert(key, index, psl);
  table.construct_value(index, value);
  return true;
}

TEMPLATE
bool MAP::assign(const key_type& key, const mapped_type& value) {
  auto [index, psl, found] = lookup_data(key);
  if (found) table.values[index] = value;
  return found;
}

TEMPLATE
auto MAP::operator[](const key_type& key) -> mapped_type& {
  auto [index, psl, found] = lookup_data(key);
  if (found) return table.values[index];
  index = insert(key, index, psl);
  table.construct_value(index);
  return table.values[index];
}

TEMPLATE
auto MAP::operator()(const key_type& key) -> mapped_type& {
  const auto [index, psl, found] = lookup_data(key);
  if (found) return table.values[index];
  throw std::invalid_argument("Failed to find the given key.");
}

TEMPLATE
auto MAP::operator()(const key_type& key) const -> const mapped_type& {
  return const_cast<map&>(*this).operator()(key);
}

TEMPLATE
auto MAP::lookup_iterator(const key_type& key) noexcept -> iterator {
  const auto [index, psl, found] = lookup_data(key);
  if (found) return {&table, index};
  return end();
}

TEMPLATE
auto MAP::lookup_iterator(const key_type& key) const noexcept
    -> const_iterator {
  const auto [index, psl, found] = lookup_data(key);
  if (found) return {&table, index};
  return end();
}

TEMPLATE
inline bool MAP::contains(const key_type& key) const noexcept {
  const auto [index, psl, found] = lookup_data(key);
  return found;
}

TEMPLATE
void MAP::erase_and_move(size_type index) {
  auto next_index = next(index);
  while (table.psl[next_index] > 1) {
    table.keys[index]   = std::move(table.keys[next_index]);
    table.values[index] = std::move(table.values[next_index]);
    table.psl[index]    = table.psl[next_index] - 1;

    index      = next_index;
    next_index = next(next_index);
  }
  table.destroy(index);
}

TEMPLATE
bool MAP::erase(const key_type& key) {
  const auto [index, psl, found] = lookup_data(key);
  if (!found) return false;
  erase_and_move(index);
  --load;
  return true;
}

TEMPLATE
void MAP::reserve(size_type size) {
  if (size <= table.size) return;
  const auto new_size = ceil_pow2(size);
  set_capacity_and_rehash(new_size);
}

TEMPLATE
void MAP::rehash(size_type count) {
  count = std::ceil(count / max_load);
  if (count <= table.size) return;
  const auto new_size = ceil_pow2(count);
  set_capacity_and_rehash(new_size);
}

TEMPLATE
void MAP::clear() {
  for (size_type i = 0; i < table.size; ++i) {
    if (!table.psl[i]) continue;
    table.destroy(i);
  }
}

TEMPLATE
template <typename pair_iterator>
void MAP::insert(pair_iterator first, pair_iterator last) {
  for (auto it = first; it != last; ++it) {
    const auto& [key, value] = *it;
    operator[](key)          = value;
  }
}

TEMPLATE
template <typename key_iterator, typename value_iterator>
void MAP::insert(key_iterator first, key_iterator last, value_iterator v) {
  for (auto it = first; it != last; ++it, ++v)
    operator[](*it) = *v;
}

TEMPLATE
MAP::map(std::initializer_list<std::pair<key_type, mapped_type>> list) {
  rehash(list.size());
  insert(list.begin(), list.end());
}

TEMPLATE
inline std::ostream& operator<<(std::ostream& os, const MAP& m) {
  using namespace std;
  os << '\n';
  for (size_t i = 0; i < m.table.size; ++i) {
    os << setw(15) << i;
    if (!m.table.psl[i]) {
      os << ' ' << setfill('-') << setw(45) << '\n' << setfill(' ');
      continue;
    }
    os << setw(15) << m.table.keys[i] << setw(15) << m.table.values[i]
       << setw(15) << m.table.psl[i] << '\n';
  }
  return os;
}

#undef TEMPLATE
#undef MAP

}  // namespace lyrahgames::robin_hood