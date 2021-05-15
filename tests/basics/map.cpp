#include <atomic>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/robin_hood/map.hpp>
#include <lyrahgames/robin_hood/version.hpp>
//
#include "log_value.hpp"

using namespace std;
using namespace lyrahgames;

template <typename k, typename m>
using hash_map = robin_hood::map<k, m>;

TEST_CASE("A robin_hood::map") {
  robin_hood::map<int, int> map{};
  using real = decltype(map)::real;

  mt19937 rng{random_device{}()};

  CHECK(map.empty());
  CHECK(map.size() == 0);
  CHECK(map.capacity() >= 8);
  CHECK(map.load_factor() == 0);

  map.set_max_load_factor(real(0.5));

  CHECK(map.max_load_factor() == real(0.5));

  SUBCASE("can insert and access values through the subscript operator '[]'.") {
    map[0] = 1;
    CHECK(map.size() == 1);
    CHECK(map[0] == 1);
    CHECK(map.size() == 1);

    map[1] = 3;
    CHECK(map.size() == 2);
    CHECK(map[1] == 3);
    CHECK(map.size() == 2);
  }

  SUBCASE("will rehash the values if the load factor is too big.") {
    map.set_max_load_factor(0.7);
    CHECK(map.max_load_factor() == 0.7f);

    CHECK(map.size() == 0);
    CHECK(map.load_factor() == 0.0f);

    const auto old_capacity = map.capacity();
    while (map.capacity() == old_capacity) {
      map[rng()] = rng();
      CHECK(map.load_factor() < map.max_load_factor());

      // cout << "{ ";
      // for (auto [k, v] : map)
      //   cout << "(" << k << ", " << v << ") ";
      // cout << "}\n";
    }

    CHECK(map.load_factor() < map.max_load_factor());
    CHECK(map.capacity() == 2 * old_capacity);
  }

  SUBCASE(
      "can access const and non-const values through the function operator "
      "'()' "
      "which throws an exception if the given key was not already inserted.") {
    map[0] = 0;
    map[1] = 4;
    map[2] = 1;
    map[3] = 5;

    CHECK(map.size() == 4);

    CHECK(map(0) == 0);
    CHECK(map(1) == 4);
    CHECK(map(2) == 1);
    CHECK(map(3) == 5);
    CHECK_THROWS_AS(map(4), std::invalid_argument);

    CHECK(map.size() == 4);

    map(2) = 3;
    CHECK(map(2) == 3);

    CHECK(map.size() == 4);

    const auto& map_ref = map;
    CHECK(map_ref(0) == 0);
    CHECK(map_ref(1) == 4);
    CHECK(map_ref(2) == 3);
    CHECK(map_ref(3) == 5);
    CHECK_THROWS_AS(map_ref(4), std::invalid_argument);

    // map_ref(2) = 5;
  }
}

SCENARIO(
    "The hash map can insert and access one or more values through "
    "iterators.") {
  GIVEN("a default constructed hash map") {
    hash_map<int, int> map{};

    GIVEN("some random vector containing values with unique keys") {
      constexpr auto count = 1000;

      mt19937 rng{random_device{}()};

      vector<int> keys(count);
      iota(begin(keys), end(keys), -count / 2);
      shuffle(begin(keys), end(keys), rng);

      vector<int> values(count);
      generate(begin(values), end(values),
               bind(uniform_int_distribution<int>{-count, count}, ref(rng)));

      vector<pair<int, int>> data{};
      for (auto i = 0; i < count; ++i)
        data.push_back({keys[i], values[i]});

      WHEN("the values are inserted") {
        map.insert(begin(data), end(data));

        THEN(
            "for every value one gets a new element in the hash map with"
            " the same key and value") {
          CHECK(map.size() == data.size());
          for (const auto& element : data) {
            CHECK_MESSAGE(map(element.first) == element.second,
                          "element = (" << element.first << ", "
                                        << element.second << ")");
          }
        }
      }

      WHEN("keys and their values are inserted separately") {
        map.insert(begin(keys), end(keys), begin(values));

        THEN(
            "for every key-value-pair one gets a new key-value-element in the "
            "hash map.") {
          CHECK(map.size() == keys.size());
          for (auto i = 0; i < keys.size(); ++i) {
            CHECK_MESSAGE(map(keys[i]) == values[i], "(keys[i], values[i]) = ("
                                                         << keys[i] << ", "
                                                         << values[i] << ")");
          }
        }
      }
    }
  }

  GIVEN("a hash map with some initial data") {
    vector<pair<int, int>> data{{1, 1}, {2, 2}, {4, 4}, {5, 5}, {10, 10}};
    hash_map<int, int>     map{};
    map.insert(begin(data), end(data));
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

    SUBCASE(
        "One can access elements through the member function 'lookup_iterator' "
        "which "
        "returns an iterator to the element if it exists and the end iterator "
        "if it does not.") {
      {
        auto [key, value] = *map.lookup_iterator(1);
        CHECK(key == 1);
        CHECK(value == 1);
      }
      {
        auto [key, value] = *map.lookup_iterator(2);
        CHECK(key == 2);
        CHECK(value == 2);
      }
      {
        auto it = map.lookup_iterator(3);
        CHECK(it == end(map));
        CHECK(it == map.end());
      }

      GIVEN("a constant reference to this hash map") {
        const auto& map_ref = map;

        SUBCASE(
            "One can access elements through the overloaded member function "
            "'lookup_iterator' which returns a constant iterator to the "
            "element if it "
            "exists and the end-iterator if it does not.") {
          {
            auto [key, value] = *map_ref.lookup_iterator(1);
            CHECK(key == 1);
            CHECK(value == 1);
          }
          {
            auto [key, value] = *map_ref.lookup_iterator(2);
            CHECK(key == 2);
            CHECK(value == 2);
          }
          {
            auto it = map_ref.lookup_iterator(3);
            CHECK(it == end(map_ref));
            CHECK(it == map_ref.end());
          }
        }
      }
    }
  }
}

SCENARIO("robin_hood::map: Use of 'reserve' and 'capacity'") {
  GIVEN("a default map") {
    robin_hood::map<int, int> map{};
    CHECK(map.capacity() == 8);
    WHEN("reserving more space than available capacity") {
      map.reserve(10);
      THEN(
          "the next bigger or equal power of two will be chosen as the new "
          "capacity of the underlying table. In this case, reallocation and "
          "rehashing are taking place and iterators and pointers are "
          "invalidated.") {
        CHECK(map.capacity() == 16);
      }
    }
    WHEN("reserving less space than available capacity") {
      map.reserve(7);
      THEN("nothing will happen.") { CHECK(map.capacity() == 8); }
    }
  }
}

SCENARIO("robin_hood::map: Initialization by Initializer List Interface") {
  WHEN("a map is initialized by an initializer list with unique keys") {
    const robin_hood::map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}, {"fourth", 4}};
    CAPTURE(map);

    THEN("all keys can be found in the map.") {
      CHECK(map.size() == 4);
      CHECK(map("first") == 1);
      CHECK(map("second") == 2);
      CHECK(map("third") == 3);
      CHECK(map("fourth") == 4);
    }
  }

  WHEN("a map is initialized by an initializer list with non-unique keys") {
    const robin_hood::map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}, {"first", 4}};
    CAPTURE(map);

    THEN("the mapped value of a non-unique key is the last given value.") {
      CHECK(map.size() == 3);
      CHECK(map("first") == 4);
      CHECK(map("second") == 2);
      CHECK(map("third") == 3);
    }
  }

  WHEN("a map is initialized by an initializer list without types") {
    robin_hood::map map{
        std::pair{"first", 1}, {"second", 2}, {"third", 3}, {"fourth", 4}};
    static_assert(
        std::same_as<decltype(map), robin_hood::map<const char*, int>>);
    CAPTURE(map);

    THEN("by CTAD all keys can be found in the map.") {
      CHECK(map.size() == 4);
      CHECK(map("first") == 1);
      CHECK(map("second") == 2);
      CHECK(map("third") == 3);
      CHECK(map("fourth") == 4);
    }
  }
}

std::ostream& operator<<(std::ostream& os, std::pair<int, int> p) {
  return os << "{" << p.first << ", " << p.second << "}";
}

SCENARIO("robin_hood::map: Emplacing Elements") {
  GIVEN("an empty hash map") {
    robin_hood::map<int, std::pair<int, int>> map{{1, {2, 3}}};
    // CAPTURE(map);
    CHECK(map.size() == 1);

    WHEN("attempting to emplace a value for an exsiting key") {
      const auto done = map.emplace(1, 4, 5);
      THEN("nothing is done at all.") {
        CHECK(!done);
        CHECK(map.size() == 1);
        CHECK(map(1).first == 2);
        CHECK(map(1).second == 3);
      }
    }

    WHEN("attempting to emplace a value for a new key") {
      const auto done = map.emplace(2, 4, 5);
      THEN(
          "a new element is added by perfect forward constructing the "
          "according value based on the given arguments.") {
        CHECK(done);
        CHECK(map.size() == 2);
        CHECK(map(2).first == 4);
        CHECK(map(2).second == 5);
      }
    }
  }
}

SCENARIO("robin_hood::map: Erasing Elements by Using Keys and Iterators") {
  GIVEN("a hash map with some initial data") {
    robin_hood::map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}, {"fourth", 4}, {"fifth", 5}};
    CAPTURE(map);
    CHECK(map.size() == 5);

    WHEN("erasing a non-existent key") {
      const auto done = map.erase("sixth");
      THEN("nothing is done at all.") {
        CHECK(!done);
        CHECK(map.size() == 5);
        CHECK(map("first") == 1);
        CHECK(map("second") == 2);
        CHECK(map("third") == 3);
        CHECK(map("fourth") == 4);
        CHECK(map("fifth") == 5);
      }
    }

    WHEN("erasing an existing key") {
      const auto done = map.erase("first");
      THEN("the element with the according key is erased from the map.") {
        CHECK(done);
        CHECK(map.size() == 4);
        CHECK_THROWS_AS(map("first"), std::invalid_argument);
        CHECK(map("second") == 2);
        CHECK(map("third") == 3);
        CHECK(map("fourth") == 4);
        CHECK(map("fifth") == 5);
      }
    }

    GIVEN("an iterator to an element inside the map") {
      auto it = map.lookup_iterator("second");
      WHEN("erasing the element given by the iterator") {
        map.erase(it);
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

SCENARIO("robin_hood::map: Static Insertion of Elements") {
  GIVEN("a map with known capacity") {
    robin_hood::map<int, int> map{};
    CAPTURE(map);

    map.set_max_load_factor(0.5);

    CHECK(map.size() == 0);
    CHECK(map.capacity() == 8);

    WHEN(
        "elements are statically inserted such that the load factor does not "
        "exceed the maximun load factor") {
      THEN("they are inserted as if 'insert' would have been called.") {
        CHECK(map.static_insert(1, 1));
        CHECK(map(1) == 1);

        CHECK(map.static_insert(2));
        CHECK(map(2) == 0);

        CHECK(map.try_static_insert(3, 3));
        CHECK(map(3) == 3);

        CHECK(!map.static_insert(1, 2));
        CHECK(map(1) == 1);

        CHECK(!map.try_static_insert(3));
        CHECK(map(3) == 3);

        CHECK(map.size() == 3);
        CHECK(map.capacity() == 8);

        AND_WHEN("the load would exceed the maximum load factor") {
          THEN("the routine throws an exception.") {
            CHECK_THROWS_AS(map.static_insert(4, 4), std::overflow_error);
            CHECK(map.size() == 3);
            CHECK(map.capacity() == 8);
          }
        }
        AND_WHEN(
            "the load would exceed the maximum load factor and one tries to "
            "statically insert a new element") {
          THEN("the routine does not succeed and returns false.") {
            CHECK(!map.try_static_insert(4, 4));
            CHECK(map.size() == 3);
            CHECK(map.capacity() == 8);
          }
        }
      }
    }
  }
}

SCENARIO("robin_hood::map: Insertion of Elements") {
  GIVEN("an empty map") {
    robin_hood::map<int, int> map{};
    CAPTURE(map);

    THEN("elements can be inserted by using 'insert'.") {
      auto done = map.insert(1, 1);
      CHECK(done);

      done = map.insert(2);
      CHECK(done);

      CHECK(map.size() == 2);
      CHECK(map(1) == 1);
      CHECK(map(2) == 0);

      WHEN("an element has already been inserted") {
        done = map.insert(1, 2);

        THEN("nothing is done at all the function returns false.") {
          CHECK(!done);
          CHECK(map.size() == 2);
          CHECK(map(1) == 1);
          CHECK(map(2) == 0);
        }
      }

      WHEN("inserting enough elements") {
        THEN(
            "the map will automatically trigger a reallocation and rehashing "
            "with a larger capacity.") {
          map.set_max_load_factor(0.5);
          CHECK(map.capacity() == 8);

          CHECK(map.insert(3, 3));
          CHECK(map(3) == 3);
          CHECK(map.size() == 3);
          CHECK(map.capacity() == 8);

          CHECK(map.insert(4, 4));
          CHECK(map(4) == 4);
          CHECK(map.size() == 4);
          CHECK(map.capacity() == 16);

          CHECK(map(1) == 1);
          CHECK(map(2) == 0);
          CHECK(map(3) == 3);
        }
      }
    }
  }
}

SCENARIO("robin_hood::map: Assigning Values to Elements") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);

    CHECK(map.size() == 3);
    CHECK(map(1) == 1);
    CHECK(map(2) == 2);
    CHECK(map(3) == 3);

    WHEN("newly assigning an already existing element") {
      auto done = map.assign(1, 2);

      THEN("the routine succeeds and returns true.") {
        CHECK(done);
        CHECK(map.size() == 3);
        CHECK(map(1) == 2);
        CHECK(map(2) == 2);
        CHECK(map(3) == 3);
      }
    }

    WHEN("newly assigning a non-existing element") {
      auto done = map.assign(4, 4);

      THEN("the routine does nothing and returns false.") {
        CHECK(!done);
        CHECK(map.size() == 3);
        CHECK(map(1) == 1);
        CHECK(map(2) == 2);
        CHECK(map(3) == 3);
      }
    }
  }
}

SCENARIO("robin_hood::map: Containing Elements") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);

    THEN("it can be checked if the map contains a given key") {
      const auto& m = map;
      CHECK(m.contains(1));
      CHECK(m.contains(2));
      CHECK(m.contains(3));
      CHECK(!m.contains(4));
      CHECK(!m.contains(5));
    }
  }
}

SCENARIO("robin_hood::map: Clear all Elements") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);

    map.reserve(16);
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

SCENARIO("robin_hood::map: Printing the Map State") {
  robin_hood::map<string, int> map{};

  cout << map << '\n';
  map["test"]  = 1;
  map["helo"]  = 2;
  map["cd"]    = 3;
  map["cp"]    = 4;
  map["ls"]    = 5;
  map["tree"]  = 6;
  map["cat"]   = 7;
  map["mkdir"] = 8;
  map["rm"]    = 9;
  map["ls"]    = 10;
  map["b"]     = 11;
  map["bdep"]  = 12;
  map["g++"]   = 13;
  map["clang"] = 14;
  map["make"]  = 15;
  map["bpkg"]  = 16;
  map["bash"]  = 17;
  map["fish"]  = 18;
  map["top"]   = 19;
  map["htop"]  = 20;
  map["git"]   = 21;
  map["vim"]   = 22;
  map["touch"] = 23;
  map["rmdir"] = 24;
  map["sudo"]  = 25;
  map["nano"]  = 26;
  cout << map << '\n';
  map.erase("bpkg");
  cout << map << '\n';

  //   for (auto [key, value] : map)
  //     cout << setw(15) << key << setw(15) << value << '\n';
}

SCENARIO("") {
  using log_value = basic_log_value<int, unique_log>;

  static_assert(xstd::generic::member_swappable<log_value>);

  log_value x, y;
  robin_hood::swap(x, y);

  reset(log_value::log);
  struct log::state state {};

  // State and log are always compared and do not have to be captured.
  // CAPTURE(state);
  // CAPTURE(log_value::log);

  CHECK(log_value::log == state);

  {
    robin_hood::map<log_value, int> map{};
    CAPTURE(map);

    map.reserve(10);
    CHECK(log_value::log == state);

    map[1] = 1;
    ++state.counters[state.construct_calls];
    ++state.counters[state.destruct_calls];
    ++state.counters[state.copy_construct_calls];
    ++state.counters[state.hash_calls];
    CHECK(log_value::log == state);

    map[2] = 2;
    ++state.counters[state.construct_calls];
    ++state.counters[state.destruct_calls];
    ++state.counters[state.copy_construct_calls];
    ++state.counters[state.hash_calls];
    CHECK(log_value::log == state);

    map[map.capacity() + 1] = 3;
    state.counters[state.construct_calls] += 1;
    state.counters[state.destruct_calls] += 2;
    state.counters[state.copy_assign_calls] += 1;
    state.counters[state.move_construct_calls] += 2;
    state.counters[state.hash_calls] += 1;
    state.counters[state.equal_calls] += 1;
    CHECK(log_value::log == state);

    map[2 * map.capacity() + 1] = 4;
    state.counters[state.construct_calls] += 1;
    state.counters[state.destruct_calls] += 2;
    state.counters[state.copy_assign_calls] += 1;
    state.counters[state.move_construct_calls] += 2;
    state.counters[state.hash_calls] += 1;
    state.counters[state.equal_calls] += 2;
    CHECK(log_value::log == state);

    map[3 * map.capacity()] = 5;
    ++state.counters[state.construct_calls];
    ++state.counters[state.destruct_calls];
    ++state.counters[state.copy_construct_calls];
    ++state.counters[state.hash_calls];
    CHECK(log_value::log == state);

    map[4 * map.capacity()] = 6;
    state.counters[state.construct_calls] += 1;
    state.counters[state.destruct_calls] += 2;  // urgument and tmp
    state.counters[state.copy_assign_calls] += 1;
    state.counters[state.move_construct_calls] += 2;
    state.counters[state.swap_calls] += 1;
    state.counters[state.hash_calls] += 1;
    state.counters[state.equal_calls] += 1;
    CHECK(log_value::log == state);
  }
  // cout << log_value::log;
  state.counters[state.destruct_calls] += 6;
  CHECK(log_value::log == state);
}

// SCENARIO("") {
//   {
//     robin_hood::map<int, test_value> map{};
//     test_value::print_stats();
//     map[1] = 1;
//     map[4] = 2;
//     map[5] = 3;
//     map[2] = 4;
//     map[9] = 5;
//     map[17] = 6;
//     map[3] = 7;
//     cout << map << '\n';
//     test_value::print_stats();
//   }
//   test_value::print_stats();
//   test_value::reset_stats();
// }
