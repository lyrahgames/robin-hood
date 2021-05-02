#include <iomanip>
#include <iostream>
#include <string>
//
#include <doctest/doctest.h>
//
#include <lyrahgames/robin_hood/map.hpp>
#include <lyrahgames/robin_hood/version.hpp>

using namespace std;
using namespace lyrahgames;

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

  for (auto [key, value] : map) {
    cout << setw(15) << key << setw(15) << value << '\n';
  }
}
