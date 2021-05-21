#include <atomic>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
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

    cout << map << endl;

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
        CHECK(false);
      }
    }
  }
}