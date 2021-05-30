#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
//
#include <lyrahgames/xstd/chrono.hpp>
//
#include <lyrahgames/robin_hood/map.hpp>
//
#include "duplication_removal.hpp"
#include "point.hpp"

using namespace std;
using namespace lyrahgames;

int main(int argc, char** argv) {
  implementation method      = implementation::lyrahgames_robin_hood_map;
  string         method_name = "lyrahgames_robin_hood_map";
  if (argc > 1) {
    const auto arg = string(argv[1]);
    if (arg == "lyrahgames_robin_hood_map")
      method = implementation::lyrahgames_robin_hood_map;
    else if (arg == "lyrahgames_robin_hood_flat_set")
      method = implementation::lyrahgames_robin_hood_flat_set;
    else if (arg == "lyrahgames_robin_hood_flat_map")
      method = implementation::lyrahgames_robin_hood_flat_map;
    else if (arg == "std_unordered_map")
      method = implementation::std_unordered_map;
    else if (arg == "naive")
      method = implementation::naive;
    else {
      cerr << "Failed to parse implementation method!\n";
      return -1;
    }
    method_name = arg;
  }

  size_t n = 100000;
  if (argc > 2) {
    n = stoi(argv[2]);
  }

  float ratio = 0.1;
  if (argc > 3) {
    ratio = stof(argv[3]);
  }
  assert((0.0 < ratio) && (ratio < 0.5));

  cout << setw(30) << "method = " << method_name << '\n'
       << setw(30) << "point count = " << setw(15) << n << '\n'
       << setw(30) << "dupication ratio = " << setw(15) << ratio << '\n'
       << flush;

  const size_t duplicated_point_count = ratio * n;
  const size_t unique_point_count     = n - duplicated_point_count;

  // Random Number Generation
  auto rng     = mt19937{random_device{}()};
  auto dist    = uniform_real_distribution<float>{};
  auto uniform = [&rng, &dist] { return dist(rng); };

  // Initialize random points.
  vector<point> points(n);
  for (auto& p : points) {
    p.x = uniform();
    p.y = uniform();
    p.z = uniform();
  }

  // Double points.
  for (size_t i = 0; i < duplicated_point_count; ++i)
    points[i] = points[n - 1 - i];

  // Manually create point set as reference.
  vector<point> ref_point_set(unique_point_count);
  for (size_t i = 0; i < unique_point_count; ++i)
    ref_point_set[i] = points[i];
  sort(begin(ref_point_set), end(ref_point_set));

  // Shuffle points.
  shuffle(begin(points), end(points), rng);

  chrono::duration<double> time{};

  switch (method) {
    case implementation::naive:
      assert(naive::duplication_count(points) == duplicated_point_count);
      time = xstd::duration(
          [&points] { points = naive::duplication_removal(points); });
      break;

    case implementation::std_unordered_map:
      assert(std_unordered_map::duplication_count(points) ==
             duplicated_point_count);
      time = xstd::duration([&points] {
        points = std_unordered_map::duplication_removal(points);
      });
      break;

    case implementation::lyrahgames_robin_hood_map:
      assert(lyrahgames_robin_hood_map::duplication_count(points) ==
             duplicated_point_count);
      time = xstd::duration([&points] {
        points = lyrahgames_robin_hood_map::duplication_removal(points);
      });
      break;

    case implementation::lyrahgames_robin_hood_flat_set:
      assert(lyrahgames_robin_hood_flat_set::duplication_count(points) ==
             duplicated_point_count);
      time = xstd::duration([&points] {
        points = lyrahgames_robin_hood_flat_set::duplication_removal(points);
      });
      break;

    case implementation::lyrahgames_robin_hood_flat_map:
      assert(lyrahgames_robin_hood_flat_map::duplication_count(points) ==
             duplicated_point_count);
      time = xstd::duration([&points] {
        points = lyrahgames_robin_hood_flat_map::duplication_removal(points);
      });
      break;
  }

  assert(points.size() == unique_point_count);
  cout << setw(30) << "time = " << setw(15) << time.count() << " s" << '\n';

  sort(begin(points), end(points));
  assert(points == ref_point_set);
}