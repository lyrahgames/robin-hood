/// @file

namespace lyrahgames::robin_hood {

#define MEMBER(X)                                                     \
  template <typename key_type, typename mapped_type, typename hasher, \
            typename equality>                                        \
  X map<key_type, mapped_type, hasher, equality>::

MEMBER(template <bool constant> struct) basic_iterator {
  using reference =
      std::conditional_t<constant,
                         std::pair<const key_type&, const mapped_type&>,
                         std::pair<const key_type&, mapped_type&>>;

  using container_pointer =
      std::conditional_t<constant, const container*, container*>;

  basic_iterator& operator++() {
    do {
      ++index;
    } while ((index < base->size) && !base->psl[index]);
    return *this;
  }
  basic_iterator operator++(int) {
    auto ip = *this;
    ++(*this);
    return ip;
  }
  reference operator*() const {
    return {base->keys[index], base->values[index]};
  }
  bool operator==(basic_iterator it) const noexcept {
    // return (base == it.base) && (index == it.index);
    return index == it.index;
  }
  bool operator!=(basic_iterator it) const noexcept {
    // return (index != it.index) || (base != it.base);
    return index != it.index;
  }

  container_pointer base{nullptr};
  size_t index{0};
};

// template <typename key_type,
//           typename mapped_type,
//           typename hasher,
//           typename equality>
// struct map<key_type, mapped_type, hasher, equality>::container {
MEMBER(struct) container {
  // psl_type should be smaller -> uint32_t or even uint16_t
  // psl will not get that long and otherwise
  // it is a bad hash implementation
  using psl_type = size_t;

  container() = default;
  container(size_t s)
      : keys{make_unique<key_type[]>(s)},
        values{make_unique<mapped_type[]>(s)},
        psl{new psl_type[s]{0}},
        size{s} {}

  container(const container&) = delete;
  container& operator=(const container&) = delete;

  container(container &&) = default;
  container& operator=(container&&) = default;

  virtual ~container() noexcept = default;

  void swap(container & c) noexcept {
    std::swap(keys, c.keys);
    std::swap(values, c.values);
    std::swap(psl, c.psl);
    std::swap(size, c.size);
  }

  std::unique_ptr<key_type[]> keys{};
  std::unique_ptr<mapped_type[]> values{};
  std::unique_ptr<psl_type[]> psl{};
  size_type size{};
};

MEMBER(inline auto) begin() noexcept -> iterator {
  for (size_t i = 0; i < table.size; ++i)
    if (table.psl[i]) return {&table, i};
  return {&table, table.size};
}

MEMBER(inline auto) begin() const noexcept -> const_iterator {
  for (size_t i = 0; i < table.size; ++i)
    if (table.psl[i]) return {&table, i};
  return {&table, table.size};
}

MEMBER(inline auto) end() noexcept -> iterator {
  return {&table, table.size};
}

MEMBER(inline auto) end() const noexcept -> const_iterator {
  return {&table, table.size};
}

MEMBER(auto) find(const key_type& key) noexcept -> iterator {
  const auto mask = table.size - size_type{1};
  size_t index = hash(key) & mask;
  size_t psl = 1;
  for (; psl <= table.psl[index]; ++psl) {
    if (equal(table.keys[index], key)) return {&table, index};
    index = (index + 1) & mask;
  }
  return end();
}

MEMBER(auto) find(const key_type& key) const noexcept -> const_iterator {
  const auto mask = table.size - size_type{1};
  size_t index = hash(key) & mask;
  size_t psl = 1;
  for (; psl <= table.psl[index]; ++psl) {
    if (equal(table.keys[index], key)) return {&table, index};
    index = (index + 1) & mask;
  }
  return end();
}

MEMBER(void) set_max_load_factor(real x) {
  assert((x <= 0) || (x >= 1));
  max_load = x;
}

MEMBER(auto)
new_key_swap_index(
    const key_type& key) const noexcept->std::pair<size_t, size_t> {
  const auto mask = table.size - size_type{1};
  size_t index = hash(key) & mask;
  size_t psl = 1;
  for (; psl <= table.psl[index]; ++psl) {
    index = (index + 1) & mask;
  }
  return {index, psl};
}

MEMBER(void) insert_key_by_swap(const key_type& key, size_t index, size_t psl) {
  if (!table.psl[index]) {
    table.keys[index] = key;
    // table.values[index] = std::move(tmp_value);
    table.psl[index] = psl;
    return;
  }

  key_type tmp_key = std::move(table.keys[index]);
  mapped_type tmp_value = std::move(table.values[index]);
  table.keys[index] = key;
  // table.values[index] = mapped_type{};
  std::swap(psl, table.psl[index]);
  ++psl;
  const auto mask = table.size - size_type{1};
  index = (index + 1) & mask;

  for (; table.psl[index]; ++psl) {
    if (psl > table.psl[index]) {
      std::swap(psl, table.psl[index]);
      std::swap(tmp_key, table.keys[index]);
      std::swap(tmp_value, table.values[index]);
    }
    index = (index + 1) & mask;
  }

  table.keys[index] = std::move(tmp_key);
  table.values[index] = std::move(tmp_value);
  table.psl[index] = psl;
}

MEMBER(void) double_capacity_and_rehash() {
  container old_table{table.size << size_type{1}};
  table.swap(old_table);
  for (size_t i = 0; i < old_table.size; ++i) {
    if (!old_table.psl[i]) continue;
    const auto [index, psl] = new_key_swap_index(old_table.keys[i]);
    insert_key_by_swap(old_table.keys[i], index, psl);
    table.values[index] = old_table.values[i];
  }
}

MEMBER(auto) operator[](const key_type& key) -> mapped_type& {
  // Try to find the element.
  const auto mask = table.size - size_type{1};
  size_t index = hash(key) & mask;
  size_t psl = 1;
  for (; psl <= table.psl[index]; ++psl) {
    if (equal(table.keys[index], key)) return table.values[index];
    index = (index + 1) & mask;
  }

  // Could not find the element. So insert it by Robin-Hood swapping.
  ++load;
  if (load >= max_load * table.size) {
    double_capacity_and_rehash();
    const auto [tmp_index, tmp_psl] = new_key_swap_index(key);
    index = tmp_index;
    psl = tmp_psl;
  }

  insert_key_by_swap(key, index, psl);
  return table.values[index];
}

MEMBER(auto) operator()(const key_type& key) -> mapped_type& {
  // Try to find the element.
  const auto mask = table.size - size_type{1};
  size_t index = hash(key) & mask;
  size_t psl = 1;
  for (; psl <= table.psl[index]; ++psl) {
    if (equal_to(table.keys[index], key)) return table.values[index];
    index = (index + 1) & mask;
  }
  // Could not find the element.
  throw std::out_of_range("Given element could not be found in hash_map.");
}

MEMBER(auto) operator()(const key_type& key) const -> const mapped_type& {
  return const_cast<map&>(*this).operator()(key);
}

MEMBER(void) erase_by_swap(size_t index) {
  const auto mask = table.size - size_type{1};
  size_t next_index = (index + 1) & mask;
  while (table.psl[next_index] > 1) {
    table.keys[index] = std::move(table.keys[next_index]);
    table.values[index] = std::move(table.values[next_index]);
    table.psl[index] = table.psl[next_index] - 1;
    index = next_index;
    next_index = (next_index + 1) & mask;
  }
  table.psl[index] = 0;
  // destroy key and value
}

MEMBER(bool) erase(const key_type& key) noexcept {
  const auto mask = table.size - size_type{1};
  size_t index = hash(key) & mask;
  size_t psl = 1;
  for (; psl <= table.psl[index]; ++psl) {
    if (equal(table.keys[index], key)) {
      erase_by_swap(index);
      --load;
      return true;
    }
    index = (index + 1) & mask;
  }
  return false;
}

#undef MEMBER

}  // namespace lyrahgames::robin_hood