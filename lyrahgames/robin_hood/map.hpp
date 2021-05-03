#pragma once
#include <cassert>
#include <cmath>
#include <functional>
#include <memory>
#include <stdexcept>
//
#include <iomanip>
#include <iostream>
//
#include <lyrahgames/robin_hood/utility.hpp>

namespace lyrahgames::robin_hood {

/// \class map map.hpp lyrahgames/robin_hood/map.hpp
template <typename key_type,
          typename mapped_type,
          typename hasher = std::hash<key_type>,
          typename equality = std::equal_to<key_type>>
class map {
  template <bool constant>
  struct basic_iterator;
  struct container;

 public:
  using real = float;
  using size_type = size_t;
  // psl_type should be smaller -> uint32_t or even uint16_t
  // psl will not get that long and otherwise
  // it is a bad hash implementation
  using psl_type = size_t;
  using iterator = basic_iterator<false>;
  using const_iterator = basic_iterator<true>;

  friend std::ostream& operator<<(std::ostream& os, const map& m) {
    using namespace std;
    for (size_t i = 0; i < m.table.size; ++i)
      os << setw(15) << m.table.keys[i] << setw(15) << m.table.values[i]
         << setw(15) << m.table.psl[i] << '\n';
    return os;
  }

  /// Checks if the map contains zero elements.
  bool empty() const { return load == 0; }

  /// Returns the count of inserted elements.
  auto size() const { return load; }

  /// Returns the maximum number of storable elements in the current table.
  /// The capacity is doubled when the the load factor exceeds the maximum load
  /// facter.
  auto capacity() const { return table.size; }

  /// Returns the current load factor of the map.
  /// The load factor is the quotient of size and capacity.
  auto load_factor() const { return real(size()) / capacity(); }

  /// Returns the maximum load factor the map is allowed to have before
  /// rehashing all elements with a bigger capacity.
  auto max_load_factor() const { return max_load; }

  /// Sets the maximum load factor the map is allowed to have before
  /// rehashing all elements with a bigger capacity.
  // real -> open_normalized<real>
  void set_max_load_factor(real x);

  auto begin() noexcept -> iterator;
  auto begin() const noexcept -> const_iterator;
  auto end() noexcept -> iterator;
  auto end() const noexcept -> const_iterator;

  auto lookup_iterator(const key_type& key) noexcept -> iterator;
  auto lookup_iterator(const key_type& key) const noexcept -> const_iterator;

  auto operator[](const key_type& key) -> mapped_type&;

  bool insert(const key_type& key, const mapped_type& value);
  bool insert_or_assign(const key_type& key, const mapped_type& value);
  bool assign(const key_type& key, const mapped_type& value);

  auto operator()(const key_type& key) -> mapped_type&;
  auto operator()(const key_type& key) const -> const mapped_type&;

  bool erase(const key_type& key) noexcept;

  /// Reserves enough memory in the underlying table by creating a new temporary
  /// table with the given size ceiled to the next power of two, rehashing all
  /// elements into it, and swapping its content with the actual table.
  void reserve(size_type size);

  /// Reserves enough memory in the underlying table such that 'count' elements
  /// could be inserted without implicitly triggering a rehash with respect to
  /// the current maximum allowed load factor. @see reserve
  void rehash(size_type count);

  void clear();
  void shrink_to_fit();

 private:
  /// Returns the ideal table index of a given key
  /// when no collision would occur.
  auto ideal_index(const key_type& key) const noexcept -> size_type;

  /// Advances the given table index by one possibly starting again at zero when
  /// runnin over table size boundary.
  auto next(size_type index) const noexcept -> size_type;

  /// If the key is contained in the map then this function returns its index,
  /// probe sequence length, and 'true'. Otherwise, it would return the index
  /// where it would have to be inserted with the according probe sequence
  /// length and 'false'.
  auto lookup_data(const key_type& key) const noexcept
      -> std::tuple<size_type, psl_type, bool>;

  /// Checks if the current load factor is bigger
  /// than the maximum allowed load factor.
  bool overloaded() const noexcept { return load >= max_load * table.size; }

  /// Assumes the given key has not already been inserted and computes table
  /// index and probe sequence length where Robin Hood swapping would have to be
  /// started.
  auto static_insert_data(const key_type& key) const noexcept
      -> std::pair<size_type, psl_type>;

  /// Inserts a new key in the map by using Robin Hood swapping algorithm.
  /// Assumes that index and psl were computed by 'insert_data'
  /// and that capacity is big enough such that map will not be overloaded.
  void static_insert(const key_type& key, size_type index, psl_type psl);

  /// Assumes the key has not been inserted and inserts it by possibly changing
  /// the capacity of the underlying table. The function returns the index
  /// where the element has been inserted.
  auto insert(const key_type& key, size_type index, psl_type psl) -> size_type;

  /// Directly sets the new size of the underlying table and rehashes all
  /// inserted elements into it. The function assumes that the given size is a
  /// positive power of two.
  void set_capacity_and_rehash(size_type size);

  /// Doubles the amount of allocated space of the underlying table and inserts
  /// all elements again.
  void double_capacity_and_rehash();

  void erase_by_swap(size_t index);

  // State
  container table{8};
  hasher hash{};
  equality equal{};
  size_type load{};
  // open_normalized<real>
  real max_load{0.8};
  // We need stats:
  // min, max index -> faster begin() and end()
  // min, max psl
};

}  // namespace lyrahgames::robin_hood

#include <lyrahgames/robin_hood/map.ipp>