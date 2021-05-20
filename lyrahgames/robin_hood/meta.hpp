#pragma once
#include <concepts>
#include <iterator>
#include <ranges>
//
#include <lyrahgames/xstd/forward.hpp>
#include <lyrahgames/xstd/meta.hpp>

namespace lyrahgames::robin_hood {

using namespace lyrahgames::xstd;

namespace generic {

using namespace lyrahgames::xstd::generic;

template <typename T>
concept value = std::destructible<T>&& std::movable<T>&& irreducible<T>;

// Keys need to be copyable. Otherwise, we are not able to do any lookups based
// on semantics even if the syntax would allow this.
template <typename T>
concept key = value<T>&& std::copyable<T>;

template <typename F, typename T>
concept hasher = irreducible<T>&& callable<F>&&
                     identical<meta::qualified_result<F>, size_t> &&
                 (meta::argument_count<F> == 1) &&
                 (identical<meta::qualified_argument<F>, T> ||
                  identical<meta::qualified_argument<F>, const T&>);

template <typename T, typename F>
concept hashability = hasher<F, T>;

template <typename F, typename T>
concept equivalence_relation =
    irreducible<T> && (std::equivalence_relation<F, T, T> ||
                       std::equivalence_relation<F, const T, const T&>);

template <typename T, typename F>
concept equivalence_relatable = equivalence_relation<F, T>;

template <typename A>
concept allocator = true;

template <typename T, typename K, typename V>
concept pair_input_iterator = std::input_iterator<T>&&  //
    requires(T it, K& k, V& v) {
  std::tie(k, v) = *it;
};

template <typename T, typename K>
concept input_iterator = std::input_iterator<T>&&  //
    requires(T it, K& k) {
  { k = *it }
  ->identical<K&>;
};

template <typename T, typename K, typename V>
concept pair_input_range = std::ranges::input_range<T>&&  //
    requires(std::ranges::range_value_t<T>& r, K& k, V& v) {
  std::tie(k, v) = r;
};

}  // namespace generic

}  // namespace lyrahgames::robin_hood