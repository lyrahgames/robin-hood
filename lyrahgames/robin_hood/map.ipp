/// @file

namespace lyrahgames::robin_hood {

// Make implementation of members manageable by providing macros
// for template parameters and class namespace.
// Try to mimic the C++ syntax to make parsing possible for other tools.
#define TEMPLATE                                                      \
  template <typename key_type, typename mapped_type, typename hasher, \
            typename equality, typename allocator>
#define MAP map<key_type, mapped_type, hasher, equality, allocator>

// TEMPLATE
// MAP::basic_key_allocator MAP::key_alloc{};
// TEMPLATE
// MAP::basic_value_allocator MAP::value_alloc{};
// TEMPLATE
// MAP::basic_psl_allocator MAP::psl_alloc{};

// TEMPLATE
// template <bool constant>
// struct MAP::basic_iterator {
//   using reference =
//       std::conditional_t<constant,
//                          std::pair<const key_type&, const mapped_type&>,
//                          std::pair<const key_type&, mapped_type&>>;

//   using container_pointer =
//       std::conditional_t<constant, const container*, container*>;

//   basic_iterator& operator++() {
//     do {
//       ++index;
//     } while ((index < base->size) && !base->psl[index]);
//     return *this;
//   }

//   basic_iterator operator++(int) {
//     auto ip = *this;
//     ++(*this);
//     return ip;
//   }

//   reference operator*() const {
//     return {base->keys[index], base->values[index]};
//   }

//   bool operator==(basic_iterator it) const noexcept {
//     // return (base == it.base) && (index == it.index);
//     return index == it.index;
//   }

//   bool operator!=(basic_iterator it) const noexcept {
//     // return (index != it.index) || (base != it.base);
//     return index != it.index;
//   }

//   // State
//   container_pointer base{nullptr};
//   size_t index{0};
// };

// TEMPLATE
// struct MAP::container {
//   container() = default;
//   container(size_t s)
//       : keys{key_allocator::allocate(key_alloc, s)},
//         values{value_allocator::allocate(value_alloc, s)},
//         psl{psl_allocator::allocate(psl_alloc, s)},
//         size{s} {
//     std::fill(psl, psl + size, 0);
//     // std::cout << "Container created with size " << size << std::endl;
//   }
//   virtual ~container() noexcept {
//     if (!psl) return;

//     // Destroy all inserted elements.
//     size_type destructions = 0;
//     for (size_type i = 0; i < size; ++i) {
//       if (!psl[i]) continue;
//       key_allocator::destroy(key_alloc, keys + i);
//       value_allocator::destroy(value_alloc, values + i);
//       ++destructions;
//     }

//     // Deallocate memory.
//     psl_allocator::deallocate(psl_alloc, psl, size);
//     value_allocator::deallocate(value_alloc, values, size);
//     key_allocator::deallocate(key_alloc, keys, size);
//     // std::cout << "Container destroyed with size " << size << " and "
//     //           << destructions << " destructions." << std::endl;
//   }

//   // No copy is allowed.
//   container(const container&) = delete;
//   container& operator=(const container&) = delete;
//   // Move is possible due to unique_ptr implementation.
//   container(container&&) = delete;
//   container& operator=(container&&) = delete;

//   // A container should provide a custom swap implementation.
//   void swap(container& c) noexcept {
//     std::swap(keys, c.keys);
//     std::swap(values, c.values);
//     std::swap(psl, c.psl);
//     std::swap(size, c.size);
//   }

//   // State
//   // std::unique_ptr<key_type[]> keys{};
//   // std::unique_ptr<mapped_type[]> values{};
//   // std::unique_ptr<psl_type[]> psl{};
//   key_type* keys{};
//   mapped_type* values{};
//   psl_type* psl{};
//   size_type size{};
// };

// TEMPLATE
// inline auto MAP::begin() noexcept -> iterator {
//   for (size_t i = 0; i < table.size; ++i)
//     if (table.psl[i]) return {&table, i};
//   return {&table, table.size};
// }

// TEMPLATE
// inline auto MAP::begin() const noexcept -> const_iterator {
//   for (size_t i = 0; i < table.size; ++i)
//     if (table.psl[i]) return {&table, i};
//   return {&table, table.size};
// }

// TEMPLATE
// inline auto MAP::end() noexcept -> iterator {
//   return {&table, table.size};
// }

// TEMPLATE
// inline auto MAP::end() const noexcept -> const_iterator {
//   return {&table, table.size};
// }

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
  auto   index = ideal_index(key);
  size_t psl   = 1;
  for (; psl <= table.psl[index]; ++psl) {
    if (equal(table.keys[index], key)) return {index, psl, true};
    index = next(index);
  }
  return {index, psl, false};
}

TEMPLATE
inline auto MAP::static_insert_data(const key_type& key) const noexcept
    -> std::pair<size_type, psl_type> {
  auto   index = ideal_index(key);
  size_t psl   = 1;
  for (; psl <= table.psl[index]; ++psl)
    index = next(index);
  return {index, psl};
}

TEMPLATE
void MAP::static_insert(const key_type& key, size_type index, psl_type psl) {
  if (!table.psl[index]) {
    // table.keys[index] = key;
    // table.values[index] = std::move(tmp_value);
    table.psl[index] = psl;
    // key_allocator::construct(key_alloc, table.keys + index, key);
    table.construct_key(index, key);
    return;
  }

  key_type    tmp_key   = std::move(table.keys[index]);
  mapped_type tmp_value = std::move(table.values[index]);
  table.keys[index]     = key;
  // table.values[index] = mapped_type{};
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

  // table.keys[index] = std::move(tmp_key);
  // table.values[index] = std::move(tmp_value);
  // key_allocator::construct(key_alloc, table.keys + index,
  // std::move(tmp_key)); value_allocator::construct(value_alloc, table.values +
  // index,
  //                            std::move(tmp_value));
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
    // static_insert(std::move(old_table.keys[i]), index, psl);
    // table.values[index] = std::move(old_table.values[i]);
    // value_allocator::construct(value_alloc, table.values + index,
    //                            std::move(old_table.values[i]));
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
  // table.values[index] = value;
  // value_allocator::construct(value_alloc, table.values + index, value);
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
  // table.values[index] = value;
  // value_allocator::construct(value_alloc, table.values + index, value);
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
  // value_allocator::construct(value_alloc, table.values + index);
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
  // key_allocator::destroy(key_alloc, table.keys + index);
  // value_allocator::destroy(value_alloc, table.values + index);
  // table.destroy_key(index);
  // table.destroy_value(index);
  // table.psl[index] = 0;
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
    // key_allocator::destroy(key_alloc, table.keys + i);
    // value_allocator::destroy(value_alloc, table.values + i);
    // table.destroy_key(i);
    // table.destroy_value(i);
    // table.psl[i] = 0;
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