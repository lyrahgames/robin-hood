#pragma once
#include <lyrahgames/robin_hood/detail/traits.hpp>

namespace lyrahgames::robin_hood::detail {

template <typename table, bool constant>
struct basic_iterator {
  using table_pointer = std::conditional_t<constant, const table*, table*>;
  using size_type     = typename traits::size_type;

  basic_iterator& operator++() noexcept {
    do {
      ++index;
    } while ((index < base->size) && base->empty(index));
    return *this;
  }

  basic_iterator operator++(int) noexcept {
    auto ip = *this;
    ++(*this);
    return ip;
  }

  auto operator*() const noexcept { return base->entry(index); }

  bool operator==(basic_iterator it) const noexcept {
    // We do not have to check base equality.
    // Comparing iterators from different instances is undefined behavior.
    // return (base == it.base) && (index == it.index);
    return index == it.index;
  }

  // State
  table_pointer base  = nullptr;
  size_type     index = 0;
};

template <typename table>
struct basic_iterator_interface {
  using iterator       = basic_iterator<table, false>;
  using const_iterator = basic_iterator<table, true>;
  using size_type      = typename traits::size_type;

  decltype(auto) that() const noexcept {
    return static_cast<const table*>(this);
  }

  decltype(auto) that() noexcept { return static_cast<table*>(this); }

  auto begin() noexcept -> iterator {
    for (size_type i = 0; i < that()->size; ++i)
      if (!that()->empty(i)) return {that(), i};
    return {that(), that()->size};
  }

  auto begin() const noexcept -> const_iterator {
    for (size_type i = 0; i < that()->size; ++i)
      if (!that()->empty(i)) return {that(), i};
    return {that(), that()->size};
  }

  auto end() noexcept -> iterator { return {that(), that()->size}; }

  auto end() const noexcept -> const_iterator { return {that(), that()->size}; }
};

}  // namespace lyrahgames::robin_hood::detail