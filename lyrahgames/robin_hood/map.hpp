#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
//
#include <iomanip>
#include <iostream>
//
#include <lyrahgames/xstd/math.hpp>
#include <lyrahgames/xstd/swap.hpp>
//
#include <lyrahgames/robin_hood/meta.hpp>
//
#include <lyrahgames/robin_hood/detail/table.hpp>

namespace lyrahgames::robin_hood {

#define TEMPLATE                                                               \
  template <generic::key Key, generic::value Value,                            \
            generic::hasher<Key>               Hasher    = std::hash<Key>,     \
            generic::equivalence_relation<Key> Equality  = std::equal_to<Key>, \
            generic::allocator                 Allocator = std::allocator<Key>>
#define MAP map<Key, Value, Hasher, Equality, Allocator>

/// \class map map.hpp lyrahgames/robin_hood/map.hpp
TEMPLATE
class map {
 public:
  // Template Parameters
  using key_type    = Key;
  using mapped_type = Value;
  using hasher      = Hasher;
  using equality    = Equality;
  using allocator   = Allocator;
  // Other Types
  using real           = double;
  using container      = detail::table<key_type, mapped_type, allocator>;
  using size_type      = typename container::size_type;
  using psl_type       = typename container::psl_type;
  using iterator       = typename container::iterator;
  using const_iterator = typename container::const_iterator;

  map() = default;

  explicit map(size_type        s,
               const hasher&    h = {},
               const equality&  e = {},
               const allocator& a = {});
  map(size_type s, const allocator& a) : map(s, hasher{}, equality{}, a) {}

  map(size_type s, const hasher& h, const allocator& a)
      : map(s, h, equality{}, a) {}

  template <generic::pair_input_iterator<key_type, mapped_type> T>
  map(T                first,
      T                last,
      size_type        s,
      const hasher&    h = {},
      const equality&  e = {},
      const allocator& a = {});

  template <generic::pair_input_iterator<key_type, mapped_type> T>
  map(T first, T last, size_type s, const allocator& a)
      : map(first, last, s, hasher{}, equality{}, a) {}

  template <generic::pair_input_iterator<key_type, mapped_type> T>
  map(T first, T last, size_type s, const hasher& h, const allocator& a)
      : map(first, last, s, h, equality{}, a) {}

  template <generic::pair_input_iterator<key_type, mapped_type> T>
  map(T                first,
      T                last,
      const hasher&    h = {},
      const equality&  e = {},
      const allocator& a = {});

  template <generic::pair_input_iterator<key_type, mapped_type> T>
  map(T first, T last, const allocator& a)
      : map(first, last, hasher{}, equality{}, a) {}

  template <generic::pair_input_iterator<key_type, mapped_type> T>
  map(T first, T last, const hasher& h, const allocator& a)
      : map(first, last, h, equality{}, a) {}

  map(std::initializer_list<std::pair<key_type, mapped_type>> list,
      size_type                                               s,
      const hasher&                                           h = {},
      const equality&                                         e = {},
      const allocator&                                        a = {})
      : map(std::begin(list), std::end(list), s, h, e, a) {}

  map(std::initializer_list<std::pair<key_type, mapped_type>> list,
      size_type                                               s,
      const allocator&                                        a)
      : map(list, s, hasher{}, equality{}, a) {}

  map(std::initializer_list<std::pair<key_type, mapped_type>> list,
      size_type                                               s,
      const hasher&                                           h,
      const allocator&                                        a)
      : map(list, s, h, equality{}, a) {}

  map(std::initializer_list<std::pair<key_type, mapped_type>> list,
      const hasher&                                           h = {},
      const equality&                                         e = {},
      const allocator&                                        a = {})
      : map(std::begin(list), std::end(list), h, e, a) {}

  map(std::initializer_list<std::pair<key_type, mapped_type>> list,
      const allocator&                                        a)
      : map(list, hasher{}, equality{}, a) {}

  map(std::initializer_list<std::pair<key_type, mapped_type>> list,
      const hasher&                                           h,
      const allocator&                                        a)
      : map(list, h, equality{}, a) {}

  explicit map(const allocator& a) : map(0, hasher{}, equality{}, a) {}

  virtual ~map() noexcept = default;

  map(const map&) = default;
  map& operator=(const map&) = default;

  map(map&&)   = default;
  map& operator=(map&&) = default;

  /// Checks if the map contains zero elements.
  bool empty() const noexcept { return load == 0; }

  /// Returns the count of inserted elements.
  auto size() const noexcept { return load; }

  /// Returns the maximum number of storable elements in the current table.
  /// The capacity is doubled when the the load factor exceeds the maximum load
  /// facter.
  auto capacity() const noexcept { return table.size; }

  /// Returns the current load factor of the map.
  /// The load factor is the quotient of size and capacity.
  auto load_factor() const noexcept { return real(size()) / capacity(); }

  /// Returns the maximum load factor the map is allowed to have before
  /// rehashing all elements with a bigger capacity.
  auto max_load_factor() const noexcept { return max_load_ratio; }

  /// Sets the maximum load factor the map is allowed to have before
  /// rehashing all elements with a bigger capacity.
  // real -> open_normalized<real>
  void set_max_load_factor(real x);

  /// Return an iterator to the beginning of the map.
  auto begin() noexcept -> iterator { return table.begin(); }

  /// Return a constant iterator to the beginning of the map.
  auto begin() const noexcept -> const_iterator { return table.begin(); }

  /// Return an iterator to the end of the map.
  auto end() noexcept -> iterator { return table.end(); }

  /// Return a constant iterator to the end of the map.
  auto end() const noexcept -> const_iterator { return table.end(); }

  /// Create an iterator pointing to an element with the given key.
  /// If this is not possible, return the end iterator.
  auto lookup_iterator(const key_type& key) noexcept -> iterator;

  /// Create a constant iterator pointing to an element with the given key.
  /// If this is not possible, return the end iterator. @see lookup_iterator
  auto lookup_iterator(const key_type& key) const noexcept -> const_iterator;

  /// Returns a reference to the mapped value of the given key. If no such
  /// element exists, an exception of type std::invalid_argument is thrown.
  auto operator()(const key_type& key) -> mapped_type&;

  /// Returns a reference to the mapped value of the given key. If no such
  /// element exists, an exception of type std::invalid_argument is thrown.
  auto operator()(const key_type& key) const -> const mapped_type&;

  /// Statically insert a given element into the map without reallocation and
  /// rehashing. If a reallocation would take place, the functions throws an
  /// exception 'std::overlow_error'. If the key has already been inserted, the
  /// function does nothing and returns false. Otherwise, it inserts the element
  /// and returns true.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void static_insert(K&& key, V&& value);

  /// Does the same as 'static_insert(K&&, V&&)' with a default constructed
  /// value type. @see 'static_insert'
  template <generic::forwardable<key_type> K>
  void static_insert(K&& key)  //
      requires std::default_initializable<mapped_type>;

  /// Insert a given element into the map with possible reallocation and
  /// rehashing. If the key has already been inserted, the
  /// function does nothing and returns false. Otherwise, it inserts the element
  /// and returns true.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void insert(K&& key, V&& value);

  /// Does the same as 'insert(K&&, V&&)' with a default constructed
  /// value type. @see 'insert'
  template <generic::forwardable<key_type> K>
  void insert(K&& key)  //
      requires std::default_initializable<mapped_type>;

  template <generic::forwardable<key_type> K, typename... arguments>
  void static_emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...>;

  /// Emplace a new element into the map by constructing its value in place.
  /// This function uses perfect forwarding construction.
  template <generic::forwardable<key_type> K, typename... arguments>
  void emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...>;

  /// Statically insert a given element into the map without reallocation and
  /// rehashing. If a reallocation would take place or if the key has already
  /// been inserted, the function does nothing and returns false. Otherwise, it
  /// inserts the element and returns true.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  bool try_static_insert(K&& key, V&& value);

  /// Does the same as 'try_static_insert(K&&, V&&)' with a default constructed
  /// value type. @see 'try_static_insert'
  template <generic::forwardable<key_type> K>
  bool try_static_insert(K&& key)  //
      requires std::default_initializable<mapped_type>;

  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  bool try_insert(K&& key, V&& value);

  template <generic::forwardable<key_type> K>
  bool try_insert(K&& key)  //
      requires std::default_initializable<mapped_type>;

  template <generic::forwardable<key_type> K, typename... arguments>
  bool try_static_emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...>;

  template <generic::forwardable<key_type> K, typename... arguments>
  bool try_emplace(K&& key, arguments&&... args)  //
      requires std::constructible_from<mapped_type, arguments...>;

  /// Insert key-value pairs given by the range [first, last) into the map.
  template <generic::pair_input_iterator<key_type, mapped_type> T>
  void insert(T first, T last);

  /// Insert given keys in the range [first, last) with their according values
  /// given in the range [v, v + (last - first)) into the map.
  template <generic::input_iterator<key_type>    T,
            generic::input_iterator<mapped_type> U>
  void insert(T first, T last, U v);

  /// Access the element given by key and assign the value to it and return true
  /// if it exists. Otherwise, do nothing and return false.
  template <generic::forwardable<mapped_type> V>
  void assign(const key_type& key, V&& value);

  /// Insert an element if it not already exists.
  /// Otherwise, assign a new value to it.
  template <generic::forwardable<key_type>    K,
            generic::forwardable<mapped_type> V>
  void insert_or_assign(K&& key, V&& value);

  /// Insert or access the element given by the key. If the key has already been
  /// inserted, the functions returns a reference to its value. Otherwise, the
  /// key will be inserted with a default initialized value to which a reference
  /// is returned.
  template <generic::forwardable<Key> K>
  auto operator[](K&& key) -> mapped_type&  //
      requires std::default_initializable<mapped_type>;

  /// Checks if the given key has been inserted into the map.
  bool contains(const key_type& key) const noexcept;

  /// If the given key has been inserted, remove the element with the given key
  /// and return 'true'. Otherwise, do nothing and return 'false'.
  bool erase(const key_type& key);

  /// Erase the element inside the map pointed to by the given iterator.
  /// The function assumes the iterator points to an existent element inside the
  /// map.
  void erase(iterator it);

  /// Erase the element inside the map pointed to by the given iterator.
  /// The function assumes the iterator points to an existent element inside the
  /// map.
  void erase(const_iterator it);

  /// Reserves enough memory in the underlying table by creating a new temporary
  /// table with the given size ceiled to the next power of two, rehashing all
  /// elements into it, and swapping its content with the actual table.
  /// In this case, all iterators and pointers become invalid.
  /// If the given size is smaller than the current table size, nothing happens.
  void reserve(size_type size);

  /// Reserves enough memory in the underlying table such that 'count' elements
  /// could be inserted without implicitly triggering a rehash with respect to
  /// the current maximum allowed load factor. @see reserve
  void rehash(size_type count);

  void clear();

  void shrink_to_fit();

  /// Output the inner state of the map by using the given output stream.
  /// This function should only be used for debugging.
  friend std::ostream& operator<<(std::ostream& os, const map& m) {
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
  // template <generic::key                       K,
  //           generic::value                     M,
  //           generic::hasher<Key>               H,
  //           generic::equivalence_relation<Key> E,
  //           generic::allocator                 A>
  // friend std::ostream& operator<<(std::ostream&             os,
  //                                 const map<K, M, H, E, A>& m);

  /// If the key is contained in the map then this function returns its index,
  /// probe sequence length, and 'true'. Otherwise, it would return the index
  /// where it would have to be inserted with the according probe sequence
  /// length and 'false'.
  auto basic_lookup_data(const key_type& key) const noexcept
      -> std::tuple<size_type, psl_type, bool>;

 private:
  /// Returns the ideal table index of a given key
  /// when no collision would occur.
  auto ideal_index(const key_type& key) const noexcept -> size_type;

  /// Advances the given table index by one possibly starting again at zero when
  /// runnin over table size boundary.
  auto next(size_type index) const noexcept -> size_type;

  /// Checks if the current number of elements is bigger or equal than the
  /// maximum allowed number of elements based on the current maximum load
  /// factor.
  bool overloaded() const noexcept { return load >= max_load; }

  /// Assumes the given key has not already been inserted and computes table
  /// index and probe sequence length where Robin Hood swapping would have to be
  /// started.
  auto basic_static_insert_data(const key_type& key) const noexcept
      -> std::pair<size_type, psl_type>;

  /// Inserts a new key in the map by using Robin Hood swapping algorithm.
  /// Assumes that index and psl were computed by 'insert_data'
  /// and that capacity is big enough such that map will not be overloaded.
  // void static_insert(const key_type& key, size_type index, psl_type psl);
  template <generic::forward_reference<key_type> K>
  void basic_static_insert(K&& key, size_type index, psl_type psl);

  /// Assumes the key has not been inserted and inserts it by possibly changing
  /// the capacity of the underlying table. The function returns the index
  /// where the element has been inserted.
  template <generic::forward_reference<key_type> K>
  auto basic_insert(K&& key, size_type index, psl_type psl) -> size_type;

  template <generic::forwardable<key_type> K>
  auto static_insert_key(K&& key) -> size_type;

  template <generic::forwardable<key_type> K>
  auto insert_key(K&& key) -> size_type;

  /// Directly sets the new size of the underlying table and rehashes all
  /// inserted elements into it. The function assumes that the given size is a
  /// positive power of two.
  void set_capacity_and_rehash(size_type size);

  /// Doubles the amount of allocated space of the underlying table and inserts
  /// all elements again.
  void double_capacity_and_rehash();

  /// Erase the element at the given table index and move the subsequent
  /// elements one step back. Abort this when an element with probe sequence
  /// length of '1' occurs.
  void basic_erase(size_type index);

 private:
  // State
  /// Functor used to compute the hash of keys.
  hasher hash{};
  /// Functor used to check equality of keys.
  equality equal{};
  /// Allocator of the underlying table.
  allocator alloc{};
  /// Underlying table storing all elements.
  container table{8, alloc};
  /// Maximum allowed load factor
  // open_normalized<real>
  real max_load_ratio{0.8};
  /// Maximum allowed number of elements with current capacity
  size_type max_load{6};
  /// Count of elements inserted into the map.
  size_type load{};
  // We need stats:
  // min, max index -> faster begin() and end()
  // min, max psl
};

template <
    std::input_iterator T,
    generic::hasher<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>
        Hasher = std::hash<std::decay_t<
            typename std::iterator_traits<T>::value_type::first_type>>,
    generic::equivalence_relation<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>
                       Equality  = std::equal_to<std::decay_t<
            typename std::iterator_traits<T>::value_type::first_type>>,
    generic::allocator Allocator = std::allocator<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>>
map(T         first,
    T         last,
    size_t    s,
    Hasher    hash  = {},
    Equality  equal = {},
    Allocator alloc = {})
    -> map<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>,
        std::decay_t<typename std::iterator_traits<T>::value_type::second_type>,
        Hasher,
        Equality,
        Allocator>;

template <std::input_iterator T, generic::allocator Allocator>
map(T first, T last, size_t s, Allocator a) -> map<
    std::decay_t<typename std::iterator_traits<T>::value_type::first_type>,
    std::decay_t<typename std::iterator_traits<T>::value_type::second_type>,
    std::hash<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>,
    std::equal_to<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>,
    Allocator>;

template <std::input_iterator                                            T,
          generic::hasher<std::decay_t<
              typename std::iterator_traits<T>::value_type::first_type>> Hasher,
          generic::allocator Allocator>
map(T         first,
    T         last,
    size_t    s,
    Hasher    h,
    Allocator a)  //
    ->map<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>,
        std::decay_t<typename std::iterator_traits<T>::value_type::second_type>,
        Hasher,
        std::equal_to<std::decay_t<
            typename std::iterator_traits<T>::value_type::first_type>>,
        Allocator>;

template <
    std::input_iterator T,
    generic::hasher<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>
        Hasher = std::hash<std::decay_t<
            typename std::iterator_traits<T>::value_type::first_type>>,
    generic::equivalence_relation<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>
                       Equality  = std::equal_to<std::decay_t<
            typename std::iterator_traits<T>::value_type::first_type>>,
    generic::allocator Allocator = std::allocator<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>>
map(T         first,
    T         last,
    Hasher    hash  = {},
    Equality  equal = {},
    Allocator alloc = {})
    -> map<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>,
        std::decay_t<typename std::iterator_traits<T>::value_type::second_type>,
        Hasher,
        Equality,
        Allocator>;

template <std::input_iterator T, generic::allocator Allocator>
map(T first, T last, Allocator a) -> map<
    std::decay_t<typename std::iterator_traits<T>::value_type::first_type>,
    std::decay_t<typename std::iterator_traits<T>::value_type::second_type>,
    std::hash<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>,
    std::equal_to<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>,
    Allocator>;

template <std::input_iterator                                            T,
          generic::hasher<std::decay_t<
              typename std::iterator_traits<T>::value_type::first_type>> Hasher,
          generic::allocator Allocator>
map(T first, T last, Hasher h, Allocator a) -> map<
    std::decay_t<typename std::iterator_traits<T>::value_type::first_type>,
    std::decay_t<typename std::iterator_traits<T>::value_type::second_type>,
    Hasher,
    std::equal_to<
        std::decay_t<typename std::iterator_traits<T>::value_type::first_type>>,
    Allocator>;

TEMPLATE
map(std::initializer_list<std::pair<Key, Value>> list,
    size_t                                       s,
    Hasher                                       hash  = {},
    Equality                                     equal = {},
    Allocator                                    alloc = {})
    ->map<Key, Value, Hasher, Equality, Allocator>;

template <generic::key Key, generic::value Value, generic::allocator Allocator>
map(std::initializer_list<std::pair<Key, Value>> list, size_t s, Allocator a)
    -> map<Key, Value, std::hash<Key>, std::equal_to<Key>, Allocator>;

template <generic::key         Key,
          generic::value       Value,
          generic::hasher<Key> Hasher,
          generic::allocator   Allocator>
map(std::initializer_list<std::pair<Key, Value>> list,
    size_t                                       s,
    Hasher                                       h,
    Allocator                                    a)  //
    ->map<Key, Value, Hasher, std::equal_to<Key>, Allocator>;

TEMPLATE
map(std::initializer_list<std::pair<Key, Value>> list,
    Hasher                                       hash  = {},
    Equality                                     equal = {},
    Allocator                                    alloc = {})
    ->map<Key, Value, Hasher, Equality, Allocator>;

template <generic::key Key, generic::value Value, generic::allocator Allocator>
map(std::initializer_list<std::pair<Key, Value>> list, Allocator a)
    -> map<Key, Value, std::hash<Key>, std::equal_to<Key>, Allocator>;

template <generic::key         Key,
          generic::value       Value,
          generic::hasher<Key> Hasher,
          generic::allocator   Allocator>
map(std::initializer_list<std::pair<Key, Value>> list, Hasher h, Allocator a)
    -> map<Key, Value, Hasher, std::equal_to<Key>, Allocator>;

/// Create map based on given arguments. This function is used as a helper
/// function to automatically deduce template parameters.
TEMPLATE
inline auto auto_map(size_t           size,
                     const Hasher&    hash  = {},
                     const Equality&  equal = {},
                     const Allocator& alloc = {}) {
  return MAP(size, hash, equal, alloc);
}

/// Create map based on given arguments. This function is used as a helper
/// function to automatically deduce template parameters.
TEMPLATE
inline auto auto_map(std::initializer_list<std::pair<Key, Value>> list,
                     const Hasher&                                hash  = {},
                     const Equality&                              equal = {},
                     const Allocator&                             alloc = {}) {
  return MAP(list, hash, equal, alloc);
}

#undef MAP
#undef TEMPLATE

}  // namespace lyrahgames::robin_hood

#include <lyrahgames/robin_hood/map.ipp>