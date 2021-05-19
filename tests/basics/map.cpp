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

SCENARIO("robin_hood::map::operator(): Accessing Values of Elements") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3},
                                  {4, 4}, {9, 5}, {13, 6}};
    CAPTURE(map);

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
        CHECK_THROWS_AS(map(7), std::invalid_argument);
        CHECK_THROWS_AS(map(5), std::invalid_argument);
        CHECK_THROWS_AS(map(-1), std::invalid_argument);
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
        CHECK_THROWS_AS(m(7), std::invalid_argument);
        CHECK_THROWS_AS(m(5), std::invalid_argument);
        CHECK_THROWS_AS(m(-1), std::invalid_argument);
      }
    }
  }

  GIVEN("a map using strings as keys") {
    robin_hood::map<string, int> map{{"first", 1}, {"second", 2}, {"third", 3}};
    CAPTURE(map);

    CHECK(map.size() == 3);

    THEN("the function operator can implicitly construct the key.") {
      CHECK(map("first") == 1);
      map("first") = 2;
      CHECK(map("first") == 2);
      CHECK(map.size() == 3);

      CHECK_THROWS_AS(map("fourth"), std::invalid_argument);
      CHECK(map.size() == 3);
    }
  }
}

SCENARIO("robin_hood::map::operator[]: Insert or Access Elements") {
  GIVEN("an empty map") {
    robin_hood::map<int, int> map{};
    CAPTURE(map);

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
    robin_hood::map<int, int> map{{1, 0}, {5, 1}, {7, 2}, {13, 3}, {3, 4}};
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
    robin_hood::map<string, int> map{{"first", 1}, {"second", 2}, {"third", 3}};
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

SCENARIO("robin_hood::map::insert: Insertion by Using Iterators") {
  GIVEN("a default map") {
    robin_hood::map<int, int> map{};

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
        map.insert(begin(data), end(data));

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
        map.insert(begin(keys), end(keys), begin(values));

        THEN(
            "for every key-value pair one gets a new key-value element in the "
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
    vector<pair<int, int>>    data{{1, 1}, {2, 2}, {4, 4}, {5, 5}, {10, 10}};
    robin_hood::map<int, int> map{};
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
  }
}

SCENARIO("robin_hood::map::lookup_iterator: Accessing Elements by Iterator") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {4, 4}, {5, 5}, {10, 10}};

    WHEN("'lookup_iterator' is used on existing keys") {
      const auto keys = {1, 2, 4, 5, 10};
      int        i    = 0;
      for (auto key : keys) {
        ++i;
        const auto it = map.lookup_iterator(key);

        THEN("an iterator is returned referencing the respective element.") {
          const auto& [k, v] = *it;
          CHECK(k == key);
          CHECK(v == i);
        }
      }
    }

    WHEN("'lookup_iterator' is used on non-existing keys") {
      const auto keys = {-1, -2, 11, 13, 8};
      for (auto key : keys) {
        const auto it = map.lookup_iterator(key);

        THEN("the end iterator is returned.") {
          CHECK(it == map.end());
          CHECK(it == end(map));
        }
      }
    }

    GIVEN("a constant reference to this map") {
      const auto& m = map;

      WHEN("'lookup_iterator' is used on existing keys") {
        const auto keys = {1, 2, 4, 5, 10};
        int        i    = 0;
        for (auto key : keys) {
          ++i;
          const auto it = m.lookup_iterator(key);

          THEN("an iterator is returned referencing the respective element.") {
            const auto [k, v] = *it;
            CHECK(k == key);
            CHECK(v == i);
          }
        }
      }

      WHEN("'lookup_iterator' is used on non-existing keys") {
        const auto keys = {-1, -2, 11, 13, 8};
        for (auto key : keys) {
          const auto it = m.lookup_iterator(key);

          THEN("the end iterator is returned.") {
            CHECK(it == m.end());
            CHECK(it == end(m));
          }
        }
      }
    }
  }
}

SCENARIO("robin_hood::map:reserve_capacity: Reserving Memory") {
  GIVEN("a default map") {
    robin_hood::map<int, int> map{};
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

SCENARIO("robin_hood::map::map: Initialization by Initializer List") {
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

SCENARIO("robin_hood::map:emplace: Emplacing Elements") {
  GIVEN("a map") {
    robin_hood::map<int, std::pair<int, int>> map{{1, {2, 3}}};
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

SCENARIO(
    "robin_hood::map::erase: Erasing Elements by Using Keys and Iterators") {
  GIVEN("a hash map with some initial data") {
    robin_hood::map<string, int> map{
        {"first", 1}, {"second", 2}, {"third", 3}, {"fourth", 4}, {"fifth", 5}};
    CAPTURE(map);
    CHECK(map.size() == 5);

    WHEN("erasing a non-existing key") {
      THEN("nothing is done at all.") {
        CHECK_THROWS_AS(map.erase("sixth"), std::invalid_argument);
        CHECK(map.size() == 5);
        CHECK(map("first") == 1);
        CHECK(map("second") == 2);
        CHECK(map("third") == 3);
        CHECK(map("fourth") == 4);
        CHECK(map("fifth") == 5);
      }
    }

    WHEN("erasing an existing key") {
      map.erase("first");

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

SCENARIO("robin_hood::map::static_insert: Static Insertion of Elements") {
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
        map.static_insert(1, 1);
        CHECK(map(1) == 1);

        map.static_insert(2);
        CHECK(map(2) == 0);

        CHECK(map.try_static_insert(3, 3));
        CHECK(map(3) == 3);

        CHECK_THROWS_AS(map.static_insert(1, 2), std::invalid_argument);
        CHECK(map(1) == 1);

        CHECK(!map.try_static_insert(3));
        CHECK(map(3) == 3);

        map.static_insert(4, 4);
        CHECK(map(4) == 4);

        CHECK(map.size() == 4);
        CHECK(map.capacity() == 8);

        AND_WHEN("the load would exceed the maximum load factor") {
          THEN("the routine throws an exception.") {
            CHECK_THROWS_AS(map.static_insert(5, 5), std::overflow_error);
            CHECK(!map.contains(5));
            CHECK_THROWS_AS(map(5), std::invalid_argument);
            CHECK(map.size() == 4);
            CHECK(map.capacity() == 8);
          }
        }
        AND_WHEN(
            "the load would exceed the maximum load factor and one tries to "
            "statically insert a new element") {
          THEN("the routine does not succeed and returns false.") {
            CHECK(!map.try_static_insert(5, 5));
            CHECK(map.size() == 4);
            CHECK(map.capacity() == 8);
          }
        }
      }
    }
  }
}

SCENARIO("robin_hood::map::insert: Insertion of Elements") {
  GIVEN("an empty map") {
    robin_hood::map<int, int> map{};
    CAPTURE(map);

    THEN("elements can be inserted by using 'insert'.") {
      map.insert(1, 1);
      map.insert(2);

      CHECK(map.size() == 2);
      CHECK(map(1) == 1);
      CHECK(map(2) == 0);

      WHEN("an element has already been inserted") {
        THEN("an exception is thrown.") {
          CHECK_THROWS_AS(map.insert(1, 2), std::invalid_argument);
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

          map.insert(3, 3);
          CHECK(map(3) == 3);
          CHECK(map.size() == 3);
          CHECK(map.capacity() == 8);

          map.insert(4, 4);
          CHECK(map(4) == 4);
          CHECK(map.size() == 4);
          CHECK(map.capacity() == 8);

          map.insert(5, 5);
          CHECK(map(5) == 5);
          CHECK(map.size() == 5);
          CHECK(map.capacity() == 16);

          CHECK(map(1) == 1);
          CHECK(map(2) == 0);
          CHECK(map(3) == 3);
          CHECK(map(4) == 4);
        }
      }
    }
  }
}

SCENARIO("robin_hood::map::assign: Assigning Values to Elements") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
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

SCENARIO("robin_hood::map::contains: Check Elements by Key") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);
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

SCENARIO("robin_hood::map::clear: Clear all Elements") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);

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

SCENARIO("robin_hood::map::operator=: Copying") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);

    WHEN("one creates a copy") {
      auto map2 = map;
      CAPTURE(map2);

      THEN(
          "the copy contains the same elements and the original has not "
          "changed.") {
        const auto tmp = map2.capacity();
        CHECK(map.capacity() == tmp);
        CHECK(map.size() == map2.size());
        CHECK(map(1) == map2(1));
        CHECK(map(2) == map2(2));
        CHECK(map(3) == map2(3));

        map.assign(1, 4);
        CHECK(map(1) == 4);
        CHECK(map2(1) == 1);

        WHEN("assigning a copy") {
          map2 = map;

          THEN("the same happens by first deleting the current content.") {
            const auto tmp = map2.capacity();
            CHECK(map.capacity() == tmp);
            CHECK(map.size() == map2.size());
            CHECK(map(1) == map2(1));
            CHECK(map(2) == map2(2));
            CHECK(map(3) == map2(3));

            map.assign(1, 5);
            CHECK(map(1) == 5);
            CHECK(map2(1) == 4);
          }
        }
      }
    }
  }
}

SCENARIO("robin_hood::map::operator=: Moving") {
  GIVEN("a map with some elements") {
    robin_hood::map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    CAPTURE(map);

    CHECK(map.capacity() == 8);
    CHECK(map.size() == 3);
    CHECK(map(1) == 1);
    CHECK(map(2) == 2);
    CHECK(map(3) == 3);

    WHEN("constructing by move") {
      auto new_map = std::move(map);
      CAPTURE(new_map);

      THEN(
          "the new map contains all elements of the old map and the old map is "
          "left in an invalid state.") {
        CHECK(new_map.capacity() == 8);
        CHECK(new_map.size() == 3);
        CHECK(new_map(1) == 1);
        CHECK(new_map(2) == 2);
        CHECK(new_map(3) == 3);

        CHECK(map.capacity() == 0);
        CHECK(map.size() == 3);

        WHEN("assigning by move") {
          map = std::move(new_map);

          THEN("the same happens.") {
            CHECK(map.capacity() == 8);
            CHECK(map.size() == 3);
            CHECK(map(1) == 1);
            CHECK(map(2) == 2);
            CHECK(map(3) == 3);

            CHECK(new_map.capacity() == 0);
            CHECK(new_map.size() == 3);
          }
        }
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

SCENARIO("robin_hood::map::basic_lookup_data: Lookup Statistics for Key Type") {
  GIVEN("a map with a key type able to log its usage and some elements") {
    // State to compare the log of log_value against.
    struct log::state state {};
    using log_value = basic_log_value<int, unique_log>;
    robin_hood::map<log_value, int> map{{1, 1}, {2, 2}, {9, 3},
                                        {0, 4}, {8, 5}, {5, 6}};
    CAPTURE(map);

    WHEN("looking up keys of already inserted elements") {
      // The creation of log_value out of integers will trigger a constructor.
      const auto keys = std::initializer_list<log_value>{1, 2, 9, 0, 8, 5};
      for (auto key : keys) {
        reset(log_value::log);
        const auto [index, psl, found] = map.basic_lookup_data(key);

        THEN(
            "no constructors, destructors, or assignments are called. Only the "
            "hash function is called once and the number of equality "
            "comparisons is equal to the probe sequence length of the given "
            "element in the map.") {
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
        const auto [index, psl, found] = map.basic_lookup_data(key);

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

SCENARIO(
    "lyrahgames::robin_hood::map::static_insert: "
    "Static Insertion of Elements"
    "by Using lvalue Keys and with Specific Values") {
  GIVEN("an empty map with known capacity and maximum load factor") {
    robin_hood::map<string, int> map{};
    CAPTURE(map);

    map.set_max_load_factor(0.5);
    map.reserve_capacity(8);

    CHECK(map.capacity() == 8);
    CHECK(map.size() == 0);

    WHEN(
        "inserting elements that do not exist such that the maximum "
        "load factor is not exceeded") {
      const auto keys =
          std::initializer_list<string>{"first", "second", "third"};

      THEN("all elements have been inserted without changing capacity.") {
        for (const auto& key : keys)
          map.static_insert(key, map.size());

        CHECK(map.size() == keys.size());
        CHECK(map.capacity() == 8);

        int i = 0;
        for (const auto& key : keys) {
          CHECK(map(key) == i);
          ++i;
        }
      }
    }
  }

  GIVEN("a map with known capacity and maximum load factor and some elements") {
    robin_hood::map<string, int> map{
        {"first", 0}, {"second", 1}, {"third", 2}, {"fourth", 3}};
    map.set_max_load_factor(0.5);
    CAPTURE(map);
    CAPTURE(map.max_load_factor());

    CHECK(map.capacity() == 8);
    CHECK(map.size() == 4);

    WHEN(
        "inserting elements that would exceed the maximum load "
        "factor and therefore trigger a reallocation") {
      const auto keys = std::initializer_list<string>{"fifth", "sixth"};

      THEN("an exception is thrown and the element is not inserted.") {
        for (const auto& key : keys) {
          CHECK_THROWS_AS(map.static_insert(key, map.size()),
                          std::overflow_error);
        }

        for (const auto& key : keys) {
          CHECK_THROWS_AS(map(key), std::invalid_argument);
          CHECK(!map.contains(key));
        }

        CHECK(map.size() == 4);
        CHECK(map.capacity() == 8);
      }
    }

    WHEN("inserting a key that has already been inserted") {
      const auto keys = std::initializer_list<string>{"third", "first"};

      THEN("the function returns false and does nothing.") {
        for (const auto& key : keys)
          CHECK_THROWS_AS(map.static_insert(key, map.size()),
                          std::invalid_argument);
        CHECK(map.capacity() == 8);
        CHECK(map.size() == 4);
        CHECK(map("first") == 0);
        CHECK(map("second") == 1);
        CHECK(map("third") == 2);
      }
    }
  }
}

SCENARIO("robin_hood::map::static_insert: Statistics for Key Type") {
  // State to compare the log of log_value against.
  struct log::state state {};
  using log_value = basic_log_value<int, unique_log>;

  GIVEN(
      "an empty map with a key type able to log its usage and a trivial "
      "hasher") {
    auto map = robin_hood::auto_map<log_value, int>(
        {}, [](const log_value& x) -> size_t {
          std::scoped_lock lock{x.log.access_mutex};
          ++x.log.state.counters[x.log.state.hash_calls];
          return x.value;
        });
    map.reserve_capacity(16);
    CAPTURE(map);

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
    auto map = robin_hood::auto_map<log_value, int>(
        {{1, 0}, {2, 1}, {7, 2}, {11, 3}, {64, 4}, {32, 5}},
        [](const log_value& x) -> size_t {
          std::scoped_lock lock{x.log.access_mutex};
          ++x.log.state.counters[x.log.state.hash_calls];
          return x.value;
        });
    map.reserve_capacity(16);
    CAPTURE(map);

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

        THEN("additionally two equals are needed.") {
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

        THEN("additionally three equals are needed.") {
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
            "additionally a temporary key is move constructed and destroyedat "
            "the end. The swapped element is move constructed in its new"
            "place. The given key is copied into the existing location.") {
          state.counters[state.copy_assign_calls]    = 1;
          state.counters[state.move_construct_calls] = 2;
          state.counters[state.destruct_calls]       = 1;
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

        THEN("additionally swap has to be called.") {
          state.counters[state.copy_assign_calls]    = 1;
          state.counters[state.move_construct_calls] = 2;
          state.counters[state.destruct_calls]       = 1;
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

        THEN("additionally equal and swap have to be called.") {
          state.counters[state.copy_assign_calls]    = 1;
          state.counters[state.move_construct_calls] = 2;
          state.counters[state.destruct_calls]       = 1;
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
          state.counters[state.move_construct_calls] = 2;
          state.counters[state.destruct_calls]       = 1;
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
          state.counters[state.move_construct_calls] = 2;
          state.counters[state.destruct_calls]       = 2;
          state.counters[state.hash_calls]           = 1;
          state.counters[state.equal_calls]          = 2;
          state.counters[state.swap_calls]           = 1;
          CHECK(log_value::log == state);
        }
      }
    }
  }
}

SCENARIO("robin_hood::auto_map") {
  SUBCASE("Size-Based") {
    auto map =
        robin_hood::auto_map<int, int>(0, [](int x) -> size_t { return x; });
    CAPTURE(map);

    CHECK(map.capacity() == 8);
    CHECK(map.size() == 0);
  }

  SUBCASE("Initializer List") {
    auto map = robin_hood::auto_map<string, int>({{"first", 1}, {"second", 2}});
    CAPTURE(map);

    CHECK(map.capacity() == 8);
    CHECK(map.size() == 2);
    CHECK(map("first") == 1);
    CHECK(map("second") == 2);
  }
}

SCENARIO("robin_hood::map: unique_ptr as Value Types") {
  random_device rng{};
  vector<int>   keys{};
  int           n = 24;

  GIVEN("an empty map with 'unique_ptr' as value type") {
    robin_hood::map<int, unique_ptr<int>> map{};
    CAPTURE(map);

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
            map.erase(keys[i]);

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