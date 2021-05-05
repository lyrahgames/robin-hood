namespace lyrahgames::robin_hood::detail {

// Make implementation of members manageable by providing macros
// for template parameters and class namespace.
// Try to mimic the C++ syntax to make parsing possible for other tools.
#define TEMPLATE template <std::destructible K, std::destructible V, typename A>
#define TABLE    table<K, V, A>

TEMPLATE
template <bool constant>
struct TABLE::basic_iterator {
  using reference =
      std::conditional_t<constant,
                         std::pair<const key_type&, const value_type&>,
                         std::pair<const key_type&, value_type&>>;
  using table_pointer = std::conditional_t<constant, const table*, table*>;

  basic_iterator& operator++() noexcept {
    do {
      ++index;
    } while ((index < base->size) && !base->valid(index));
    return *this;
  }

  basic_iterator operator++(int) noexcept {
    auto ip = *this;
    ++(*this);
    return ip;
  }

  reference operator*() const noexcept {
    return {base->keys[index], base->values[index]};
  }

  bool operator==(basic_iterator it) const noexcept {
    // We do not have to check base equality.
    // Comparing iterators from different instances is undefined behavior.
    // return (base == it.base) && (index == it.index);
    return index == it.index;
  }

  bool operator!=(basic_iterator it) const noexcept {
    // We do not have to check base equality.
    // Comparing iterators from different instances is undefined behavior.
    // return (index != it.index) || (base != it.base);
    return index != it.index;
  }

  // State
  table_pointer base  = nullptr;
  size_type     index = 0;
};

TEMPLATE
TABLE::table(size_type s)
    : keys{key_allocator::allocate(key_alloc, s)},
      values{value_allocator::allocate(value_alloc, s)},
      psl{psl_allocator::allocate(psl_alloc, s)},
      size{s} {
  std::fill(psl, psl + size, 0);
}

TEMPLATE
TABLE::~table() noexcept {
  if (!psl) return;
  clear();
  psl_allocator::deallocate(psl_alloc, psl, size);
  value_allocator::deallocate(value_alloc, values, size);
  key_allocator::deallocate(key_alloc, keys, size);
}

TEMPLATE
TABLE::table(table&& t) noexcept {
  swap(t);
}

TEMPLATE
auto TABLE::operator=(table&& t) noexcept -> table& {
  swap(t);
  return *this;
}

TEMPLATE
inline void TABLE::swap(table& t) noexcept {
  std::swap(keys, t.keys);
  std::swap(values, t.values);
  std::swap(psl, t.psl);
  std::swap(size, t.size);
}

TEMPLATE
void TABLE::clear() noexcept {
  for (size_type i = 0; i < size; ++i) {
    if (!valid(i)) continue;
    destroy_key(i);
    destroy_value(i);
  }
}

TEMPLATE
template <typename... arguments>
inline void TABLE::construct_key(size_type index, arguments&&... args)  //
    requires std::constructible_from<key_type, arguments...> {
  key_allocator::construct(key_alloc, keys + index,
                           std::forward<arguments>(args)...);
}

TEMPLATE
inline void TABLE::destroy_key(size_type index) noexcept {
  key_allocator::destroy(key_alloc, keys + index);
}

TEMPLATE
template <typename... arguments>
inline void TABLE::construct_value(size_type index, arguments&&... args)  //
    requires std::constructible_from<value_type, arguments...> {
  value_allocator::construct(value_alloc, values + index,
                             std::forward<arguments>(args)...);
}

TEMPLATE
inline void TABLE::destroy_value(size_type index) noexcept {
  value_allocator::destroy(value_alloc, values + index);
}

TEMPLATE
inline auto TABLE::begin() noexcept -> iterator {
  for (size_type i = 0; i < size; ++i)
    if (valid(i)) return {this, i};
  return {this, size};
}

TEMPLATE
inline auto TABLE::begin() const noexcept -> const_iterator {
  for (size_type i = 0; i < size; ++i)
    if (valid(i)) return {this, i};
  return {this, size};
}

TEMPLATE
inline auto TABLE::end() noexcept -> iterator {
  return {this, size};
}

TEMPLATE
inline auto TABLE::end() const noexcept -> const_iterator {
  return {this, size};
}

#undef TABLE
#undef TEMPLATE

}  // namespace lyrahgames::robin_hood::detail