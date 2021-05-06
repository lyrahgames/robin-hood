#pragma once
#include <lyrahgames/xstd/meta.hpp>

namespace lyrahgames::robin_hood {

using namespace lyrahgames::xstd;

namespace generic {
using namespace lyrahgames::xstd::generic;

template <typename T>
concept value = std::destructible<T>&& std::movable<T>;

template <typename T>
concept key = value<T>;

template <typename F, typename T>
concept hasher =
    callable<F>&& meta::equal<meta::qualified_result<F>, size_t> &&
    (meta::argument_count<F> == 1) &&
    (meta::equal<meta::qualified_argument<F>, std::decay_t<T>> ||
     meta::equal<meta::qualified_argument<F>, const std::decay_t<T>&>);

template <typename T, typename F>
concept hashability = hasher<F, T>;

template <typename F, typename T>
concept equivalence_relation =
    std::equivalence_relation<F, std::decay_t<T>, std::decay_t<T>> ||
    std::equivalence_relation<F, const std::decay_t<T>, const std::decay_t<T>&>;

template <typename T, typename F>
concept equivalence_relatable = equivalence_relation<F, T>;

template <typename A>
concept allocator = true;

}  // namespace generic

}  // namespace lyrahgames::robin_hood