#pragma once
#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
//
#include <lyrahgames/robin_hood/meta.hpp>
//
#include <lyrahgames/robin_hood/detail/basic_iterator.hpp>
#include <lyrahgames/robin_hood/detail/flat_key_table_base.hpp>

namespace lyrahgames::robin_hood::detail {

template <generic::key       Key,
          generic::value     Value,
          generic::allocator Allocator = std::allocator<Key>>
struct key_value_table
    : public flat_key_table_base<Key, Allocator>,
      public basic_iterator_interface<key_value_table<Key, Value, Allocator>> {
  using base = flat_key_table_base<Key, Allocator>;
  using typename base::allocator;
  using typename base::key_type;
  using typename base::psl_type;
  using typename base::size_type;
  using value_type = Value;
  using entry_type = std::pair<key_type, value_type>;

  using base::alloc;
  using base::construct_key;
  using base::destroy_key;
  using base::keys;
  using base::psl;

  using basic_value_allocator = typename std::allocator_traits<
      allocator>::template rebind_alloc<value_type>;
  using value_allocator = std::allocator_traits<basic_value_allocator>;

  using iterator =
      basic_iterator<key_value_table<key_type, value_type, allocator>, false>;
  using const_iterator =
      basic_iterator<key_value_table<key_type, value_type, allocator>, true>;

  template <typename... arguments>
  void construct_value(size_type index, arguments&&... args)  //
      requires std::constructible_from<value_type, arguments...> {
    auto value_alloc = alloc;
    value_allocator::construct(value_alloc, values + index,
                               std::forward<arguments>(args)...);
  }

  void destroy_value(size_type index) noexcept {
    auto value_alloc = alloc;
    value_allocator::destroy(value_alloc, values + index);
  }

  void construct(size_type index, entry_type&& entry) {
    construct_key(index, std::move(entry.first));
    construct_value(index, std::move(entry.second));
  }

  void construct(size_type index, const entry_type& entry) {
    construct_key(index, entry.first);
    construct_value(index, entry.second);
  }

  void destroy(size_type index) noexcept {
    destroy_key(index);
    destroy_value(index);
    psl[index] = 0;
  }

  void swap(size_type index, entry_type& entry) noexcept {
    // With this, we can use custom swap routines when they are defined as
    // member functions. Otherwise, we try to use the standard.
    using xstd::swap;
    swap(keys[index], entry.first);
    swap(values[index], entry.second);
  }
  void move(size_type to, size_type from) {
    keys[to]   = std::move(keys[from]);
    values[to] = std::move(values[from]);
  }

  decltype(auto) entry(size_type index) noexcept {
    return std::pair<const key_type&, value_type&>{keys[index], values[index]};
  }

  decltype(auto) entry(size_type index) const noexcept {
    return std::pair<const key_type&, const value_type&>{keys[index],
                                                         values[index]};
  }

  decltype(auto) extraction(size_type index) noexcept {
    return std::pair<key_type, value_type>{std::move(keys[index]),
                                           std::move(values[index])};
  }

  value_type* values = nullptr;
};

}  // namespace lyrahgames::robin_hood::detail
