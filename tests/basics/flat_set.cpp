#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/robin_hood/flat_set.hpp>
//
#include "log_value.hpp"

using namespace std;
using namespace lyrahgames;

template <typename T>
inline bool contains(const std::vector<T>& keys, const T& key) {
  for (const auto& k : keys)
    if (key == k) return true;
  return false;
}

SCENARIO("robin_hood::flat_set::insert") {
  random_device rng{};
  using key_type = decltype(rng());
  vector<key_type> keys{};
  size_t           n = 20;

  GIVEN("an empty set") {
    robin_hood::flat_set<key_type> set{};
    CAPTURE(set);
    CAPTURE(set.data());

    CHECK(set.empty());

    THEN("") {
      for (size_t i = 0; i < n; ++i) {
        const auto rnd = rng();
        try {
          set.insert(rnd);
        } catch (std::invalid_argument&) {
          CHECK(contains(keys, rnd));
        }
        keys.push_back(rnd);
      }
      n = keys.size();

      CHECK(set.size() == n);
      for (int i = 0; i < n; ++i) {
        CHECK(set.contains(keys[i]));
        CHECK(set(keys[i]));
      }

      THEN("") {
        for (int i = 0; i < n / 2; ++i)
          set.remove(keys[i]);
        for (int i = 0; i < n / 2; ++i)
          CHECK_THROWS_AS(set.remove(keys[i]), invalid_argument);

        CHECK(set.size() == (n - n / 2));
        for (int i = 0; i < n / 2; ++i) {
          CHECK(!set.contains(keys[i]));
          CHECK(!set(keys[i]));
        }
        for (int i = n / 2; i < n; ++i) {
          CHECK(set.contains(keys[i]));
          CHECK(set(keys[i]));
        }

        THEN("") {
          for (size_t i = 0; i < n / 2; ++i)
            set[keys[i]];

          CHECK(set.size() == n);
          for (int i = 0; i < n; ++i) {
            CHECK(set.contains(keys[i]));
            CHECK(set(keys[i]));
          }
        }
      }
    }
  }
}

SCENARIO("robin_hood::flat_set::lookup_data: Lookup Statistics for Key Type") {
  GIVEN("a set with a key type able to log its usage and some elements") {
    // State to compare the log of log_value against.
    struct log::state state {};
    using log_value = basic_log_value<int, unique_log>;
    robin_hood::flat_set<log_value> set{};
    set[1][2][9][0][8][5];

    CAPTURE(set);
    CAPTURE(set.data());

    WHEN("looking up keys of already inserted elements") {
      // The creation of log_value out of integers will trigger a constructor.
      const auto keys = std::initializer_list<log_value>{1, 2, 9, 0, 8, 5};
      for (auto key : keys) {
        reset(log_value::log);
        const auto [index, psl, found] = set.lookup_data(key);

        THEN(
            "no constructors, destructors, or assignments are called. Only the "
            " hash function is called once and the number of equality "
            "comparisons is equal to the probe sequence length of the given "
            "element in the set.") {
          CHECK(found);
          state.counters[state.hash_calls]  = 1;
          state.counters[state.equal_calls] = psl;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN("looking up non-existing keys") {
      // The creation of log_value out of integers will trigger a constructor.
      const auto keys = std::initializer_list<log_value>{3, 7, 18};
      for (auto key : keys) {
        reset(log_value::log);
        const auto [index, psl, found] = set.lookup_data(key);

        THEN(
            "no constructors, destructors, or assignments are called. Only the "
            "hash function is called once and the number equality comparisons "
            "is equal to the probe sequence length minus one of the given "
            "element.") {
          CHECK(!found);
          state.counters[state.hash_calls]  = 1;
          state.counters[state.equal_calls] = psl - 1;
          CHECK(log_value::log == state);
        }
      }
    }
  }
}

SCENARIO("robin_hood::flat_set::static_insert: Statistics for Key Type") {
  // State to compare the log of log_value against.
  struct log::state state {};
  using log_value = basic_log_value<int, unique_log>;

  GIVEN(
      "an empty set with a key type able to log its usage and a trivial "
      "hasher") {
    auto set = robin_hood::auto_flat_set<log_value>(
        16, [](const log_value& x) -> size_t {
          std::scoped_lock lock{x.log.access_mutex};
          ++x.log.state.counters[x.log.state.hash_calls];
          return x.value;
        });
    CAPTURE(set);
    CAPTURE(set.data());

    WHEN("statically inserting elements without collision by lvalue") {
      const auto keys = std::initializer_list<log_value>{1, 2, 7, 11, 64};
      for (auto key : keys) {
        reset(log_value::log);
        set.static_insert(key);

        THEN("only the hash function and the copy constructor are called.") {
          state.counters[state.copy_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN("statically inserting elements without collision by rvalue") {
      const auto keys = std::initializer_list<log_value>{1, 2, 7, 11, 64};
      for (auto key : keys) {
        reset(log_value::log);
        set.static_insert(std::move(key));

        THEN("only the hash function and the move constructor are called.") {
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN("statically inserting elements without collision by construction") {
      auto keys = {1, 2, 7, 11, 64};
      for (auto key : keys) {
        reset(log_value::log);
        set.static_insert(key);

        THEN("only the hash function and the move constructor are called.") {
          state.counters[state.construct_calls]      = 1;
          state.counters[state.destruct_calls]       = 1;
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }
  }
}