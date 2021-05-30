#include <atomic>
#include <iomanip>
#include <iostream>
#include <random>
#include <span>
#include <string>
#include <vector>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/robin_hood/flat_map.hpp>
#include <lyrahgames/robin_hood/version.hpp>
//
#include "log_value.hpp"

using namespace std;
using namespace lyrahgames;

SCENARIO("robin_hood::flat_map::operator(): Accessing Values of Elements") {
  GIVEN("a map with some elements") {
    robin_hood::flat_map<int, int> map{{1, 1}, {2, 2}, {3, 3},
                                       {4, 4}, {9, 5}, {13, 6}};
    CAPTURE(map);
    CAPTURE(map.data());

    CHECK(map.size() == 6);

    WHEN("using the function operator with an existing key") {
      THEN("returns a reference to the respective value.") {
        CHECK(map(1) == 1);
        CHECK(map(2) == 2);
        CHECK(map(3) == 3);
        CHECK(map(4) == 4);
        CHECK(map(9) == 5);
        CHECK(map(13) == 6);

        CHECK(map.size() == 6);

        map[13] = 13;
        CHECK(map(13) == 13);
        CHECK(map[13] == 13);

        CHECK(map.size() == 6);
      }
    }

    WHEN("using the function operator with a non-existing key") {
      THEN("it throws an exception") {
        CHECK_THROWS_AS(map(7), invalid_argument);
        CHECK_THROWS_AS(map(5), invalid_argument);
        CHECK_THROWS_AS(map(-1), invalid_argument);
        CHECK(map.size() == 6);
      }
    }

    GIVEN("a constant reference to this map") {
      const auto& m = map;

      THEN("the same can be done without changing the values.") {
        CHECK(m(1) == 1);
        CHECK(m(2) == 2);
        CHECK(m(3) == 3);
        CHECK(m(4) == 4);
        CHECK(m(9) == 5);
        CHECK(m(13) == 6);
        CHECK_THROWS_AS(m(7), invalid_argument);
        CHECK_THROWS_AS(m(5), invalid_argument);
        CHECK_THROWS_AS(m(-1), invalid_argument);
      }
    }
  }

  GIVEN("a map using strings as keys") {
    robin_hood::flat_map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}};
    CAPTURE(map);

    CHECK(map.size() == 3);

    THEN("the function operator can implicitly construct the key.") {
      CHECK(map("first") == 1);
      map("first") = 2;
      CHECK(map("first") == 2);
      CHECK(map.size() == 3);

      CHECK_THROWS_AS(map("fourth"), invalid_argument);
      CHECK(map.size() == 3);
    }
  }
}

SCENARIO("robin_hood::flat_map::operator[]: Insert or Access Elements") {
  GIVEN("an empty map") {
    robin_hood::flat_map<int, int> map{};
    CAPTURE(map);
    CAPTURE(map.data());

    CHECK(map.empty());
    CHECK(map.size() == 0);

    WHEN("calling the subscript operator with a non-existing key") {
      const auto keys = {1, 5, 3, 16, 27, 18};
      for (auto key : keys)
        map[key];

      THEN(
          "a new element is inserted into the map with the given key and a "
          "default initialized value.") {
        CHECK(map.size() == keys.size());
        for (auto key : keys) {
          CHECK(map.contains(key));
          CHECK(map(key) == 0);
        }
      }
    }

    WHEN("assigning the subscript operator with a non-existing key") {
      const auto keys = {1, 5, 3, 16, 27, 18};
      for (auto key : keys)
        map[key] = map.size();

      THEN(
          "a new element is inserted into the map with the given key and a "
          "default initialized value. Afterwards, the reference to the created "
          "value is overriden.") {
        int i = 0;
        for (auto key : keys) {
          CHECK(map.contains(key));
          CHECK(map(key) == i);
          ++i;
        }
      }
    }
  }

  GIVEN("a map with some elements") {
    robin_hood::flat_map<int, int> map{{1, 0}, {5, 1}, {7, 2}, {13, 3}, {3, 4}};
    CAPTURE(map);

    CHECK(map.size() == 5);

    WHEN("using the subcript operator with an existing key") {
      THEN("it returns a reference to the respective value.") {
        CHECK(map[1] == 0);
        CHECK(map[5] == 1);
        CHECK(map[7] == 2);
        CHECK(map[13] == 3);
        CHECK(map[3] == 4);

        CHECK(map.size() == 5);

        map[7] = 7;
        CHECK(map(7) == 7);
        CHECK(map[7] == 7);

        CHECK(map.size() == 5);

        map[13] = 13;
        CHECK(map(13) == 13);
        CHECK(map[13] == 13);

        CHECK(map.size() == 5);
      }
    }
  }

  GIVEN("a map using strings as keys") {
    robin_hood::flat_map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}};
    CAPTURE(map);

    CHECK(map.size() == 3);

    THEN("the subscript operator can implicitly construct the key.") {
      CHECK(map["first"] == 1);
      map["first"] = 2;
      CHECK(map["first"] == 2);
      CHECK(map.size() == 3);

      map["fourth"] = 4;
      CHECK(map["fourth"] == 4);
      CHECK(map.size() == 4);
    }
  }
}

SCENARIO("robin_hood::flat_map::insert: Insertion by Using Iterators") {
  GIVEN("a default map") {
    robin_hood::flat_map<int, int> map{};

    GIVEN("some random vector containing values with unique keys") {
      constexpr auto count = 1000;
      mt19937        rng{random_device{}()};

      vector<int> keys(count);
      iota(begin(keys), end(keys), -count / 2);
      shuffle(begin(keys), end(keys), rng);

      vector<int> values(count);
      generate(begin(values), end(values),
               bind(uniform_int_distribution<int>{-count, count}, ref(rng)));

      vector<pair<int, int>> data{};
      for (auto i = 0; i < count; ++i)
        data.push_back({keys[i], values[i]});

      WHEN("the paired elements are inserted") {
        map.insert(span{begin(data), end(data)});

        THEN(
            "for every value one gets a new element in the hash map with"
            " the same key and value.") {
          CHECK(map.size() == data.size());
          for (const auto& element : data) {
            CHECK_MESSAGE(map(element.first) == element.second,
                          "element = (" << element.first << ", "
                                        << element.second << ")");
          }
        }
      }

      WHEN("keys and their values are inserted separately") {
        map.insert(span{begin(keys), end(keys)},
                   span{begin(values), end(values)});

        THEN(
            "for every key-value pair one gets a new key-value element in the"
            "hash map.") {
          CHECK(map.size() == keys.size());
          for (auto i = 0; i < keys.size(); ++i) {
            CHECK_MESSAGE(map(keys[i]) == values[i], "(keys[i], values[i])=("
                                                         << keys[i] << ", "
                                                         << values[i] << ")");
          }
        }
      }
    }
  }

  GIVEN("a hash map with some initial data") {
    vector<pair<int, int>> data{{1, 1}, {2, 2}, {4, 4}, {5, 5}, {10, 10}};
    robin_hood::flat_map<int, int> map{};
    map.insert(span{begin(data), end(data)});
    vector<pair<int, int>> read{};

    WHEN("one iterates with iterators and helper functions") {
      for (auto it = begin(map); it != end(map); ++it)
        read.push_back({(*it).first, (*it).second});

      THEN("every element in the hash map is reached in an undefined order") {
        sort(begin(read), end(read));
        CHECK(read == data);
      }
    }

    WHEN("one iterates with iterators and member functions") {
      for (auto it = map.begin(); it != map.end(); ++it)
        read.push_back({(*it).first, (*it).second});

      THEN("every element in the hash map is reached in an undefined order") {
        sort(begin(read), end(read));
        CHECK(read == data);
      }
    }

    WHEN("one iterates with a range-based for loop") {
      for (auto [key, value] : map)
        read.push_back({key, value});

      THEN("every element in the hash map is reached in an undefined order") {
        sort(begin(read), end(read));
        CHECK(read == data);
      }
    }

    GIVEN("a constant reference to this hash map") {
      const auto& map_ref = map;

      WHEN("one iterates with iterators and helper functions") {
        for (auto it = begin(map_ref); it != end(map_ref); ++it) {
          auto [key, value] = *it;
          read.push_back({key, value});
        }

        THEN("every element in the hash map is reached in an undefined order") {
          sort(begin(read), end(read));
          CHECK(read == data);
        }
      }

      WHEN("one iterates with iterators and member functions") {
        for (auto it = map_ref.begin(); it != map_ref.end(); ++it) {
          auto [key, value] = *it;
          read.push_back({key, value});
        }

        THEN("every element in the hash map is reached in an undefined order") {
          sort(begin(read), end(read));
          CHECK(read == data);
        }
      }

      WHEN("one iterates with a range-based for loop") {
        for (auto [key, value] : map_ref)
          read.push_back({key, value});

        THEN("every element in the hash map is reached in an undefined order") {
          sort(begin(read), end(read));
          CHECK(read == data);
        }
      }
    }
  }
}

SCENARIO("robin_hood::flat_map::lookup: Accessing Elements by Iterator") {
  GIVEN("a map with some elements") {
    robin_hood::flat_map<int, int> map{
        {1, 1}, {2, 2}, {4, 4}, {5, 5}, {10, 10}};

    WHEN("'lookup' is used on existing keys") {
      const auto keys = {1, 2, 4, 5, 10};
      int        i    = 0;
      for (auto key : keys) {
        ++i;
        const auto it = map.lookup(key);

        THEN("an iterator is returned referencing the respective element.") {
          const auto& [k, v] = *it;
          CHECK(k == key);
          CHECK(v == i);
        }
      }
    }

    WHEN("'lookup' is used on non-existing keys") {
      const auto keys = {-1, -2, 11, 13, 8};
      for (auto key : keys) {
        const auto it = map.lookup(key);

        THEN("the end iterator is returned.") {
          CHECK(it == map.end());
          CHECK(it == end(map));
        }
      }
    }

    GIVEN("a constant reference to this map") {
      const auto& m = map;

      WHEN("'lookup' is used on existing keys") {
        const auto keys = {1, 2, 4, 5, 10};
        int        i    = 0;
        for (auto key : keys) {
          ++i;
          const auto it = m.lookup(key);

          THEN("an iterator is returned referencing the respective element.") {
            const auto [k, v] = *it;
            CHECK(k == key);
            CHECK(v == i);
          }
        }
      }

      WHEN("'lookup' is used on non-existing keys") {
        const auto keys = {-1, -2, 11, 13, 8};
        for (auto key : keys) {
          const auto it = m.lookup(key);

          THEN("the end iterator is returned.") {
            CHECK(it == m.end());
            CHECK(it == end(m));
          }
        }
      }
    }
  }
}

SCENARIO("robin_hood::flat_map:reserve_capacity: Reserving Memory") {
  GIVEN("a default map") {
    robin_hood::flat_map<int, int> map{};
    CHECK(map.capacity() == 8);
    WHEN("reserving more space than available capacity") {
      map.reserve_capacity(10);
      THEN(
          "the next bigger or equal power of two will be chosen as the new "
          "capacity of the underlying table. In this case, reallocation and "
          "rehashing are taking place and iterators and pointers are "
          "invalidated.") {
        CHECK(map.capacity() == 16);
      }
    }
    WHEN("reserving less space than available capacity") {
      map.reserve_capacity(7);
      THEN("nothing will happen.") { CHECK(map.capacity() == 8); }
    }
  }
}

SCENARIO("robin_hood::flat_map::flat_map: Initialization by Initializer List") {
  WHEN("a map is initialized by an initializer list with unique keys") {
    const robin_hood::flat_map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}, {"fourth", 4}};
    CAPTURE(map);
    CAPTURE(map.data());

    THEN("all keys can be found in the map.") {
      CHECK(map.size() == 4);
      CHECK(map("first") == 1);
      CHECK(map("second") == 2);
      CHECK(map("third") == 3);
      CHECK(map("fourth") == 4);
    }
  }

  WHEN("a map is initialized by an initializer list with non-unique keys") {
    const robin_hood::flat_map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}, {"first", 4}};
    CAPTURE(map);
    CAPTURE(map.data());

    THEN("the mapped value of a non-unique key is the last given value.") {
      CHECK(map.size() == 3);
      CHECK(map("first") == 4);
      CHECK(map("second") == 2);
      CHECK(map("third") == 3);
    }
  }

  WHEN("a map is initialized by an initializer list without types") {
    robin_hood::flat_map map{
        std::pair{"first", 1}, {"second", 2}, {"third", 3}, {"fourth", 4}};
    static_assert(
        std::same_as<decltype(map), robin_hood::flat_map<const char*, int>>);
    CAPTURE(map);
    CAPTURE(map.data());

    THEN("by CTAD all keys can be found in the map.") {
      CHECK(map.size() == 4);
      CHECK(map("first") == 1);
      CHECK(map("second") == 2);
      CHECK(map("third") == 3);
      CHECK(map("fourth") == 4);
    }
  }
}

SCENARIO(
    "robin_hood::flat_map::remove: Erasing Elements by Using Keys and "
    "Iterators") {
  GIVEN("a hash map with some initial data") {
    robin_hood::flat_map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}, {"fourth", 4}, {"fifth", 5}};
    CAPTURE(map);
    CHECK(map.size() == 5);

    WHEN("erasing a non-existing key") {
      THEN("nothing is done at all.") {
        CHECK_THROWS_AS(map.remove("sixth"), std::invalid_argument);
        CHECK(map.size() == 5);
        CHECK(map("first") == 1);
        CHECK(map("second") == 2);
        CHECK(map("third") == 3);
        CHECK(map("fourth") == 4);
        CHECK(map("fifth") == 5);
      }
    }

    WHEN("erasing an existing key") {
      map.remove("first");

      THEN("the element with the according key is erased from the map.") {
        CHECK(map.size() == 4);
        CHECK_THROWS_AS(map("first"), std::invalid_argument);
        CHECK(map("second") == 2);
        CHECK(map("third") == 3);
        CHECK(map("fourth") == 4);
        CHECK(map("fifth") == 5);
      }
    }

    GIVEN("an iterator to an element inside the map") {
      auto it = map.lookup("second");
      WHEN("erasing the element given by the iterator") {
        map.remove(it);
        THEN("the element pointed to by the iterator is erased.") {
          CHECK(map.size() == 4);
          CHECK(map("first") == 1);
          CHECK_THROWS_AS(map("second"), std::invalid_argument);
          CHECK(map("third") == 3);
          CHECK(map("fourth") == 4);
          CHECK(map("fifth") == 5);
        }
      }
    }
  }
}

SCENARIO("robin_hood::flat_map::assign: Assigning Values to Elements") {
  GIVEN("a map with some elements") {
    robin_hood::flat_map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);

    CHECK(map.size() == 3);
    CHECK(map(1) == 1);
    CHECK(map(2) == 2);
    CHECK(map(3) == 3);

    WHEN("newly assigning an already existing element") {
      map.assign(1, 2);

      THEN("the routine succeeds.") {
        CHECK(map.size() == 3);
        CHECK(map(1) == 2);
        CHECK(map(2) == 2);
        CHECK(map(3) == 3);
      }
    }

    WHEN("newly assigning a non-existing element") {
      THEN("the routine throws an exception.") {
        CHECK_THROWS_AS(map.assign(4, 4), std::invalid_argument);
        CHECK(map.size() == 3);
        CHECK(map(1) == 1);
        CHECK(map(2) == 2);
        CHECK(map(3) == 3);
      }
    }
  }
}

SCENARIO("robin_hood::flat_map::contains: Check Elements by Key") {
  GIVEN("a map with some elements") {
    robin_hood::flat_map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);
    CAPTURE(map.data());
    const auto& m = map;

    THEN(
        "it can be checked if the map contains an element with the given key") {
      CHECK(m.contains(1));
      CHECK(m.contains(2));
      CHECK(m.contains(3));
      CHECK(!m.contains(4));
      CHECK(!m.contains(5));
    }
  }
}

SCENARIO("robin_hood::flat_map::clear: Clear all Elements") {
  GIVEN("a map with some elements") {
    robin_hood::flat_map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);
    CAPTURE(map.data());

    map.reserve_capacity(16);
    CHECK(map.capacity() == 16);
    CHECK(map.size() == 3);
    CHECK(map(1) == 1);
    CHECK(map(2) == 2);
    CHECK(map(3) == 3);

    WHEN("clearing the map") {
      map.clear();

      THEN("no element is left but the capacity is the same.") {
        CHECK(map.capacity() == 16);
        CHECK(map.size() == 0);
        CHECK(!map.contains(1));
        CHECK(!map.contains(2));
        CHECK(!map.contains(3));
      }
    }
  }
}

SCENARIO("robin_hood::flat_map: unique_ptr as Value Types") {
  random_device rng{};
  vector<int>   keys{};
  int           n = 24;

  GIVEN("an empty map with 'unique_ptr' as value type") {
    robin_hood::flat_map<int, unique_ptr<int>> map{};
    CAPTURE(map);
    CAPTURE(map.data());

    THEN("unique pointers can be inserted by using move.") {
      for (int i = 0; i < n; ++i) {
        const auto rnd = rng();
        // Manage doubled keys.
        try {
          map.insert(rnd, make_unique<int>(i));
        } catch (std::invalid_argument&) {
          continue;
        }
        keys.push_back(rnd);
      }
      n = keys.size();

      CHECK(map.size() == n);
      for (int i = 0; i < n; ++i) {
        CHECK(map.contains(keys[i]));
        CHECK(*map(keys[i]) == i);
      }

      THEN("new values can be assigned through the function operator.") {
        vector<int> values(n);
        for (int i = 0; i < n; ++i) {
          const auto rnd = rng();
          map(keys[i])   = make_unique<int>(rnd);
          values[i]      = rnd;
        }

        CHECK(map.size() == n);
        for (int i = 0; i < n; ++i)
          CHECK(*map(keys[i]) == values[i]);

        THEN("erasing some elements ís possible.") {
          for (int i = 0; i < n / 2; ++i)
            map.remove(keys[i]);

          CHECK(map.size() == (n - n / 2));
          for (int i = 0; i < n / 2; ++i) {
            CHECK(!map.contains(keys[i]));
            CHECK_THROWS_AS(map(keys[i]), invalid_argument);
          }
          for (int i = n / 2; i < n; ++i) {
            CHECK(map.contains(keys[i]));
            CHECK(*map(keys[i]) == values[i]);
          }

          THEN(
              "by using the subscript operator values can be inserted or "
              "assigned by using move assignments.") {
            for (int i = 0; i < n; ++i) {
              const auto rnd = rng();
              map[keys[i]]   = make_unique<int>(rnd);
              values[i]      = rnd;
            }

            CHECK(map.size() == keys.size());
            for (int i = 0; i < n; ++i) {
              CHECK(map.contains(keys[i]));
              CHECK(*map(keys[i]) == values[i]);
            }

            THEN("using swap allows to get back the ownership.") {
              for (int i = 0; i < n; ++i) {
                const auto rnd = rng();
                auto       tmp = make_unique<int>(rnd);
                swap(map(keys[i]), tmp);
                values[i] = rnd;
              }

              CHECK(map.size() == keys.size());
              for (int i = 0; i < n; ++i) {
                CHECK(map.contains(keys[i]));
                CHECK(*map(keys[i]) == values[i]);
              }
            }
          }
        }
      }
    }
  }
}

SCENARIO("robin_hood::flat_map::static_insert: Statistics for Key Type") {
  // State to compare the log of log_value against.
  struct log::state state {};
  using log_value = basic_log_value<int, unique_log>;

  GIVEN(
      "an empty map with a key type able to log its usage and a trivial "
      "hasher") {
    auto map = robin_hood::auto_flat_map<log_value, int>(
        {}, [](const log_value& x) -> size_t {
          std::scoped_lock lock{x.log.access_mutex};
          ++x.log.state.counters[x.log.state.hash_calls];
          return x.value;
        });
    map.reserve_capacity(16);
    CAPTURE(map);
    CAPTURE(map.data());

    WHEN("statically inserting elements without collision by lvalue") {
      const auto keys = std::initializer_list<log_value>{1, 2, 7, 11, 64};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key, map.size());

        THEN("only the hash function and the copy constructor are called.") {
          state.counters[state.copy_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN("default statically inserting elements without collision by lvalue") {
      const auto keys = std::initializer_list<log_value>{1, 2, 7, 11, 64};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key);

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
        map.static_insert(std::move(key), map.size());

        THEN("only the hash function and the move constructor are called.") {
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN("default statically inserting elements without collision by rvalue") {
      const auto keys = std::initializer_list<log_value>{1, 2, 7, 11, 64};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(std::move(key));

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
        map.static_insert(key, map.size());

        THEN("only the hash function and the move constructor are called.") {
          state.counters[state.construct_calls]      = 1;
          state.counters[state.destruct_calls]       = 1;
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN(
        "default statically inserting elements without collision by "
        "construction") {
      auto keys = {1, 2, 7, 11, 64};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key);

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

  GIVEN(
      "a map with a key type able to log its usage and a trivial hasher "
      "containing some elements") {
    auto map = robin_hood::auto_flat_map<log_value, int>(
        {{1, 0}, {2, 1}, {7, 2}, {11, 3}, {64, 4}, {32, 5}},
        [](const log_value& x) -> size_t {
          std::scoped_lock lock{x.log.access_mutex};
          ++x.log.state.counters[x.log.state.hash_calls];
          return x.value;
        });
    map.reserve_capacity(16);
    CAPTURE(map);
    CAPTURE(map.data());

    WHEN("statically inserting elements with one collision by lvalue") {
      auto keys = std::initializer_list<log_value>{23, 27};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key, map.size());

        THEN("only hash, equal and copy are called.") {
          state.counters[state.copy_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN("statically inserting elements with two collisions by lvalue") {
      map.static_insert(23, map.size());
      map.static_insert(27, map.size());
      auto keys = std::initializer_list<log_value>{39, 43};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key, map.size());

        THEN("two comparisons are needed.") {
          state.counters[state.copy_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 2;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN("statically inserting elements with three collisions by lvalue") {
      map.static_insert(23, map.size());
      map.static_insert(27, map.size());
      map.static_insert(39, map.size());
      map.static_insert(43, map.size());
      auto keys = std::initializer_list<log_value>{55, 59};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key, map.size());

        THEN("three comparisons are needed.") {
          state.counters[state.copy_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 3;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN(
        "statically inserting elements with one collision and one swap by "
        "lvalue") {
      map.static_insert(8, map.size());
      map.static_insert(12, map.size());
      auto keys = std::initializer_list<log_value>{23, 27};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key, map.size());

        THEN(
            "the swapped element is move constructed in its new"
            "place. The given key is copied into the existing location.") {
          state.counters[state.copy_assign_calls]    = 1;
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN(
        "statically inserting elements with one collision and two swaps by "
        "lvalue") {
      map.static_insert(8, map.size());
      map.static_insert(9, map.size());
      map.static_insert(12, map.size());
      map.static_insert(13, map.size());
      auto keys = std::initializer_list<log_value>{23, 27};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key, map.size());

        THEN("swap has to be called once.") {
          state.counters[state.copy_assign_calls]    = 1;
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 1;
          state.counters[state.swap_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN(
        "statically inserting elements with two collisions and two swaps by "
        "lvalue") {
      auto keys = std::initializer_list<log_value>{128};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(key, map.size());

        THEN("two comparisons and one swap have to be called.") {
          state.counters[state.copy_assign_calls]    = 1;
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 2;
          state.counters[state.swap_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN(
        "statically inserting elements with two collisions and two swaps by "
        "rvalue") {
      auto keys = std::initializer_list<log_value>{128};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(std::move(key), map.size());

        THEN("instead of a copy assignment the move assignment is called.") {
          state.counters[state.move_assign_calls]    = 1;
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 2;
          state.counters[state.swap_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }

    WHEN(
        "statically inserting elements with two collisions and two swaps by "
        "construction") {
      auto keys = {128};
      for (auto key : keys) {
        reset(log_value::log);
        map.static_insert(std::move(key), map.size());

        THEN("the log_value has to be constructed and destroyed.") {
          state.counters[state.construct_calls]      = 1;
          state.counters[state.move_assign_calls]    = 1;
          state.counters[state.move_construct_calls] = 1;
          state.counters[state.destruct_calls]       = 1;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 2;
          state.counters[state.swap_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }
  }
}

SCENARIO("robin_hood::flat_map:emplace: Emplacing Elements") {
  GIVEN("a map") {
    robin_hood::flat_map<int, std::pair<int, int>> map{{1, {2, 3}}};
    // CAPTURE(map);
    CHECK(map.size() == 1);

    WHEN("attempting to emplace a value for an exsiting key") {
      THEN("an exception is thrown.") {
        CHECK_THROWS_AS(map.emplace(1, 4, 5), std::invalid_argument);
        CHECK(map.size() == 1);
        CHECK(map(1).first == 2);
        CHECK(map(1).second == 3);
      }
    }

    WHEN("attempting to emplace a value for a new key") {
      map.emplace(2, 4, 5);
      THEN(
          "a new element is added by perfect forward constructing the "
          "according value based on the given arguments.") {
        CHECK(map.size() == 2);
        CHECK(map(2).first == 4);
        CHECK(map(2).second == 5);
      }
    }
  }
}