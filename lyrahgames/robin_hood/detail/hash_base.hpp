#pragma once
#include <cassert>
//
#include <lyrahgames/xstd/math.hpp>

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

  hash_base() = default;

  hash_base(size_type s, real m, hasher h, equality e, allocator a)
      : table{0, a}, hash{h}, equal{e}, max_load_ratio{m} {
    assert((0 < m) && (m < 1));
    reserve(std::max(size_type{1}, s));
  }

  hash_base(size_type s, hasher h, equality e, allocator a)
      : hash_base(s, 0.8, h, e, a) {}

  /// Returns the ideal hash index of the given key
  /// if there would be no collision.
  auto hash_index(const key_type& key) const noexcept -> size_type {
    const auto mask = table.size - size_type{1};
    return hash(key) & mask;
  }

  /// Advance the given index to the underlying table by one and return it.
  auto next(size_type index) const noexcept -> size_type {
    const auto mask = table.size - size_type{1};
    return (index + size_type{1}) & mask;
  }

  /// Checks if the current load factor of the table has reached the maximum
  /// possible load factor until a reallocation has to be done.
  bool overloaded() const noexcept {
    return load >= size_type(std::floor(max_load_ratio * table.size));
  }

  /// If the key is contained in the table then this function returns its index,
  /// probe sequence length, and 'true'. Otherwise, it would return the index
  /// where it would have to be inserted with the according probe sequence
  /// length and 'false'.
  auto lookup_data(const key_type& key) const noexcept
      -> std::tuple<size_type, psl_type, bool> {
    auto index = hash_index(key);
    auto psl   = psl_type{1};
    for (; psl < table.psl(index); ++psl)
      index = next(index);
    for (; psl == table.psl(index); ++psl) {
      if (equal(table.key(index), key)) return {index, psl, true};
      index = next(index);
    }
    return {index, psl, false};
  }

  /// Assumes the given key has not already been inserted and computes table
  /// index and probe sequence length where Robin Hood swapping would have to be
  /// started.
  auto static_insert_data(const key_type& key) const noexcept
      -> std::pair<size_type, psl_type> {
    auto index = hash_index(key);
    auto psl   = psl_type{1};
    for (; psl <= table.psl(index); ++psl)
      index = next(index);
    return {index, psl};
  }

  /// Assumes the table entry with the given index is not empty and moves the
  /// current and succeeding values along the chain by using Robin Hood
  /// swapping. The first empty entry will be move constructed. After this
  /// operation the original index can be move assigned.
  void prepare_insert(size_type index) {
    auto p = table.psl(index) + psl_type{1};
    auto i = next(index);
    for (; !table.empty(i); ++p) {
      if (p > table.psl(i)) {
        table.swap(i, index);
        swap(p, table.psl(i));
      }
      i = next(i);
    }
    table.psl(i) = p;
    table.move_construct(i, index);
  }

  /// Inserts a new key in the table by using Robin Hood swapping algorithm.
  /// Assumes that index and psl were computed by 'lookup_data'
  /// and that capacity is big enough such that map will not be overloaded.
  template <generic::forward_reference<key_type> K>
  void basic_static_insert_key(size_type index, psl_type psl, K&& key) {
    ++load;

    if (table.empty(index)) {
      table.psl(index) = psl;
      table.construct_key(index, std::forward<K>(key));
      return;
    }
    prepare_insert(index);
    table.psl(index) = psl;
    table.key(index) = std::forward<K>(key);
  }

  /// Directly sets the new size of the underlying table and rehashes all
  /// inserted elements into it. The function assumes that the given size is a
  /// positive power of two.
  void reallocate_and_rehash(size_type c) {
    container old_table{c, table.alloc};
    table.swap(old_table);
    for (size_type i = 0; i < old_table.size; ++i) {
      if (old_table.empty(i)) continue;
      const auto [index, psl] = static_insert_data(old_table.key(i));
      if (!table.empty(index)) prepare_insert(index);
      table.move_construct_or_assign(index, psl, old_table.index_iterator(i));
    }
  }

  /// Erase the element at the given table index and move the subsequent
  /// elements one step back. Abort this when an element with probe sequence
  /// length of '1' occurs. Assumes the table entry referenced by the given
  /// index is not empty.
  void basic_remove(size_type index) {
    auto next_index = next(index);
    while (table.psl(next_index) > 1) {
      table.move(index, next_index);
      table.psl(index) = table.psl(next_index) - 1;

      index      = next_index;
      next_index = next(next_index);
    }
    table.destroy(index);
    --load;
  }

  /// Doubles the amount of allocated space of the underlying table and inserts
  /// all elements again.
  void double_capacity_and_rehash() { reallocate_and_rehash(table.size << 1); }

  void reserve_capacity(size_type size) {
    size = std::max(min_capacity, size);
    if (size <= table.size) return;
    size = ceil_pow2(size);
    reallocate_and_rehash(size);
  }

  void reserve(size_type count) {
    count = std::ceil(count / max_load_ratio);
    reserve_capacity(count);
  }

  /// Sets the maximum load factor and possibly triggers a reallocation.
  /// The function assumes that the given factor lies in (0,1).
  void set_max_load_factor(real x) {
    assert((x > 0) || (x < 1));
    max_load_ratio = x;
    // Reserve capacity for the number of inserted elements
    // or at least one element.
    // reserve(load ? load : 1);
    reserve(std::max(load, size_t(1)));
  }

  template <generic::forward_reference<key_type> K>
  auto basic_insert_key(size_type index, psl_type psl, K&& key) -> size_type {
    if (overloaded()) {
      double_capacity_and_rehash();
      const auto [i, p] = static_insert_data(key);

      index = i;
      psl   = p;
    }
    basic_static_insert_key(index, psl, std::forward<K>(key));
    return index;
  }

  /// Inserts a given key into table without doing reallocation or checking for
  /// overload. The function assumes that by inserting the given key the maximum
  /// load factor will not be exceeded. It returns the key index and a boolean
  /// indicating if it has already been inserted (false) or was inserted at that
  /// point (true).
  template <generic::forwardable<key_type> K>
  auto nocheck_static_insert_key(K&& key) -> std::pair<size_type, bool> {
    // This makes sure key is constructed
    // if it is not a direct forward reference.
    decltype(auto) k = forward_construct<key_type>(std::forward<K>(key));
    auto [index, psl, found] = lookup_data(k);
    if (found) return {index, false};
    basic_static_insert_key(index, psl, std::forward<decltype(k)>(k));
    return {index, true};
  }

  /// Does the same as 'nocheck_static_insert_key' but additionally checks if an
  /// overload would occur and abort the process by returning the size of the
  /// table and false.
  template <generic::forwardable<key_type> K>
  auto try_static_insert_key(K&& key) -> std::pair<size_type, bool> {
    if (overloaded()) return {table.size, false};
    return nocheck_static_insert_key(std::forward<K>(key));
  }

  template <generic::forwardable<key_type> K>
  auto static_insert_key(K&& key) -> size_type {
    // This makes sure key is constructed if it is not a direct forward
    // reference.
    decltype(auto) k = forward_construct<key_type>(std::forward<K>(key));
    auto [index, psl, found] = lookup_data(k);
    if (found)
      throw std::invalid_argument(
          "Failed to insert element that already exists!");
    if (overloaded())
      throw std::overflow_error("Failed to statically insert given element!");
    basic_static_insert_key(index, psl, std::forward<decltype(k)>(k));
    return index;
  }

  template <generic::forwardable<key_type> K>
  auto try_insert_key(K&& key) -> std::pair<size_type, bool> {
    // This makes sure key is constructed if it is not a direct forward
    // reference.
    decltype(auto) k = forward_construct<key_type>(std::forward<K>(key));
    auto [index, psl, found] = lookup_data(k);
    if (found) return {index, false};
    index = basic_insert_key(index, psl, std::forward<decltype(k)>(k));
    return {index, true};
  }

  template <generic::forwardable<key_type> K>
  auto insert_key(K&& key) -> size_type {
    // This makes sure key is constructed if it is not a direct forward
    // reference.
    decltype(auto) k = forward_construct<key_type>(std::forward<K>(key));
    auto [index, psl, found] = lookup_data(k);
    if (found)
      throw std::invalid_argument(
          "Failed to insert element that already exists!");
    index = basic_insert_key(index, psl, std::forward<decltype(k)>(k));
    return index;
  }

  bool try_remove(const key_type& key) {
    const auto [index, psl, found] = lookup_data(key);
    if (!found) return false;
    basic_remove(index);
    return true;
  }

  void remove(const key_type& key) {
    const auto [index, psl, found] = lookup_data(key);
    if (!found)
      throw std::invalid_argument("Failed to remove non-existing key!");
    basic_remove(index);
  }

  void remove(iterator it) {
    assert((it.base == &table) && !table.empty(it.index));
    basic_remove(it.index);
  }

  void remove(const_iterator it) {
    assert((it.base == &table) && !table.empty(it.index));
    basic_remove(it.index);
  }

  bool empty() const noexcept { return load == 0; }

  auto size() const noexcept { return load; }

  auto capacity() const noexcept { return table.size; }

  auto load_factor() const noexcept { return real(size()) / capacity(); }

  auto max_load_factor() const noexcept { return max_load_ratio; }

  bool contains(const key_type& key) const noexcept {
    const auto [index, psl, found] = lookup_data(key);
    return found;
  }

  auto lookup(const key_type& key) noexcept -> iterator {
    const auto [index, psl, found] = lookup_data(key);
    if (found) return {&table, index};
    return table.end();
  }

  auto lookup(const key_type& key) const noexcept -> const_iterator {
    const auto [index, psl, found] = lookup_data(key);
    if (found) return {&table, index};
    return table.end();
  }

  void clear() {
    load = 0;
    table.clear();
  }

  static constexpr size_type min_capacity = 8;

  container table{min_capacity, allocator{}};
  hasher    hash{};
  equality  equal{};
  size_type load           = 0;
  real      max_load_ratio = 0.8;
};

}  // namespace lyrahgames::robin_hood::detail