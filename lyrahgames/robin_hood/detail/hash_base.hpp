#pragma once

namespace lyrahgames::robin_hood::detail {

template <typename Table,
          generic::hasher<typename Table::key_type>               Hasher,
          generic::equivalence_relation<typename Table::key_type> Equality>
struct hash_base {
  using container      = Table;
  using key_type       = typename container::key_type;
  using allocator      = typename container::allocator;
  using hasher         = Hasher;
  using equality       = Equality;
  using real           = double;
  using size_type      = typename container::size_type;
  using psl_type       = typename container::psl_type;
  using iterator       = typename container::iterator;
  using const_iterator = typename container::const_iterator;

  auto ideal_index(const key_type& key) const noexcept -> size_type {
    const auto mask = table.size - size_type{1};
    return hash(key) & mask;
  }

  auto next(size_type index) const noexcept -> size_type {
    const auto mask = table.size - size_type{1};
    return (index + size_type{1}) & mask;
  }

  bool overloaded() const noexcept {
    return load >= size_type(std::floor(max_load_ratio * table.size));
  }

  auto basic_lookup_data(const key_type& key) const noexcept
      -> std::tuple<size_type, psl_type, bool> {
    auto index = ideal_index(key);
    auto psl   = psl_type{1};
    for (; psl <= table.psl[index]; ++psl) {
      if (equal(table.keys[index], key)) return {index, psl, true};
      index = next(index);
    }
    return {index, psl, false};
  }

  auto basic_static_insert_data(const key_type& key) const noexcept
      -> std::pair<size_type, psl_type> {
    auto index = ideal_index(key);
    auto psl   = psl_type{1};
    for (; psl <= table.psl[index]; ++psl)
      index = next(index);
    return {index, psl};
  }

  template <generic::forward_reference<key_type> K>
  void basic_static_insert(K&& key, size_type index, psl_type psl) {
    ++load;

    if (!table.psl[index]) {
      table.psl[index] = psl;
      table.construct_key(index, std::forward<K>(key));
      return;
    }

    auto tmp          = table.extraction(index);
    table.keys[index] = std::forward<K>(key);
    std::swap(psl, table.psl[index]);
    ++psl;
    index = next(index);

    for (; table.psl[index]; ++psl) {
      if (psl > table.psl[index]) {
        std::swap(psl, table.psl[index]);
        table.swap(index, tmp);
      }
      index = next(index);
    }

    table.construct(index, std::move(tmp));
    table.psl[index] = psl;
  }

  void basic_reallocate_and_rehash(size_type c) {
    container old_table{c, table.alloc};

    table.swap(old_table);
    load = 0;

    for (size_type i = 0; i < old_table.size; ++i) {
      if (!old_table.psl[i]) continue;
      const auto [index, psl] = basic_static_insert_data(old_table.keys[i]);
      basic_static_insert(std::move(old_table.keys[i]), index, psl);
      // table.construct_value(index, std::move(old_table.values[i]));
    }
  }

  void double_capacity_and_rehash() {
    basic_reallocate_and_rehash(table.size << 1);
  }

  void reserve_capacity(size_type size) {
    // size = std::max(min_capacity, size);
    if (size <= table.size) return;
    size = ceil_pow2(size);
    basic_reallocate_and_rehash(size);
  }

  void reserve(size_type count) {
    count = std::ceil(count / max_load_ratio);
    reserve_capacity(count);
  }

  void set_max_load_factor(real x) {
    assert((x > 0) || (x < 1));
    max_load_ratio = x;
    // Reserve capacity for the number of inserted elements
    // or at least one element.
    // reserve(load ? load : 1);
    reserve(std::max(load, size_t(1)));
  }

  template <generic::forward_reference<key_type> K>
  auto basic_insert(K&& key, size_type index, psl_type psl) -> size_type {
    if (overloaded()) {
      double_capacity_and_rehash();
      const auto [i, p] = basic_static_insert_data(key);

      index = i;
      psl   = p;
    }
    basic_static_insert(std::forward<K>(key), index, psl);
    return index;
  }

  template <generic::forwardable<key_type> K>
  auto static_insert_key(K&& key) -> size_type {
    // This makes sure key is constructed if it is not a direct forward
    // reference.
    decltype(auto) k = forward_construct<key_type>(std::forward<K>(key));
    auto [index, psl, found] = basic_lookup_data(k);
    if (found)
      throw std::invalid_argument(
          "Failed to insert element that already exists!");
    if (overloaded())
      throw std::overflow_error("Failed to statically insert given element!");
    basic_static_insert(std::forward<decltype(k)>(k), index, psl);
    return index;
  }

  template <generic::forwardable<key_type> K>
  auto insert_key(K&& key) -> size_type {
    // This makes sure key is constructed if it is not a direct forward
    // reference.
    decltype(auto) k = forward_construct<key_type>(std::forward<K>(key));
    auto [index, psl, found] = basic_lookup_data(k);
    if (found)
      throw std::invalid_argument(
          "Failed to insert element that already exists!");
    index = basic_insert(std::forward<decltype(k)>(k), index, psl);
    return index;
  }

  void basic_remove(size_type index) {
    auto next_index = next(index);
    while (table.psl[next_index] > 1) {
      table.move(index, next_index);
      table.psl[index] = table.psl[next_index] - 1;

      index      = next_index;
      next_index = next(next_index);
    }
    table.destroy(index);
    --load;
  }

  void remove(const key_type& key) {
    const auto [index, psl, found] = basic_lookup_data(key);
    if (!found)
      throw std::invalid_argument("Failed to remove non-existing key!");
    basic_remove(index);
  }

  void remove(iterator it) {
    assert((it.base == &table) && table.valid(it.index));
    basic_remove(it.index);
  }

  void remove(const_iterator it) {
    assert((it.base == &table) && table.valid(it.index));
    basic_remove(it.index);
  }

  bool empty() const noexcept { return load == 0; }

  auto size() const noexcept { return load; }

  auto capacity() const noexcept { return table.size; }

  auto load_factor() const noexcept { return real(size()) / capacity(); }

  auto max_load_factor() const noexcept { return max_load_ratio; }

  bool contains(const key_type& key) const noexcept {
    const auto [index, psl, found] = basic_lookup_data(key);
    return found;
  }

  auto lookup_iterator(const key_type& key) noexcept -> iterator {
    const auto [index, psl, found] = basic_lookup_data(key);
    if (found) return {&table, index};
    return table.end();
  }

  auto lookup_iterator(const key_type& key) const noexcept -> const_iterator {
    const auto [index, psl, found] = basic_lookup_data(key);
    if (found) return {&table, index};
    return table.end();
  }

  container table{8, allocator{}};
  hasher    hash{};
  equality  equal{};
  size_type load           = 0;
  real      max_load_ratio = 0.8;
};

}  // namespace lyrahgames::robin_hood::detail