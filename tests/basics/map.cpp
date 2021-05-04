#include <iomanip>
#include <iostream>
#include <random>
#include <string>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/robin_hood/map.hpp>
#include <lyrahgames/robin_hood/version.hpp>

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
    hash_map<int, int> map{};
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

// SCENARIO("The hash map can be initialized by initializer lists.") {
//   WHEN("an initializer list with unique keys is used") {
//     // hash_map<int, int> map{{1, 5}, {-1, 2}, {8, 4}, {5, -4}, {-3, -1}};
//     // THEN("every key-value-pair can is inserted.") {
//     //   CHECK(size(map) == 5);
//     //   CHECK(map(1) == 5);
//     //   CHECK(map(-1) == 2);
//     //   CHECK(map(8) == 4);
//     //   CHECK(map(5) == -4);
//     //   CHECK(map(-3) == -1);
//     // }
//   }

//   // WHEN("an initializer list with non-unique keys is used") {
//   //   hash_map map{{1, 5}, {-1, 2}, {1, 4}, {5, -4}, {5, -1}};
//   //   THEN("the mapped value of non-unique keys is one of the given
//   values.") {
//   //     CHECK(size(map) == 3);
//   //     CHECK((map(1) - 5) * (map(1) - 4) == 0);
//   //     CHECK(map(-1) == 2);
//   //     CHECK((map(5) + 4) * (map(5) + 1) == 0);
//   //   }
//   // }
// }

TEST_CASE("Default map construction.") {
  robin_hood::map<string, int> map{};

  cout << map << '\n';
  map["test"] = 1;
  map["helo"] = 2;
  map["cd"] = 3;
  map["cp"] = 4;
  map["ls"] = 5;
  map["tree"] = 6;
  map["cat"] = 7;
  map["mkdir"] = 8;
  map["rm"] = 9;
  map["ls"] = 10;
  map["b"] = 11;
  map["bdep"] = 12;
  map["g++"] = 13;
  map["clang"] = 14;
  map["make"] = 15;
  map["bpkg"] = 16;
  map["bash"] = 17;
  map["fish"] = 18;
  map["top"] = 19;
  map["htop"] = 20;
  map["git"] = 21;
  map["vim"] = 22;
  map["touch"] = 23;
  map["rmdir"] = 24;
  map["sudo"] = 25;
  map["nano"] = 26;
  cout << map << '\n';
  map.erase("bpkg");
  cout << map << '\n';

  for (auto [key, value] : map)
    cout << setw(15) << key << setw(15) << value << '\n';
}
