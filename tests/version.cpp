#include <iomanip>
#include <iostream>
//
#include <lyrahgames/robin_hood/version.hpp>

using namespace std;

int main() {
  cout << setw(40) << "LYRAHGAMES_ROBIN_HOOD_VERSION = "  //
       << setw(40) << LYRAHGAMES_ROBIN_HOOD_VERSION << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_VERSION_STR = " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_VERSION_STR << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_VERSION_ID = " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_VERSION_ID << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_VERSION_FULL= " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_VERSION_FULL << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_VERSION_MAJOR = " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_VERSION_MAJOR << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_VERSION_MINOR = " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_VERSION_MINOR << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_VERSION_PATCH = " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_VERSION_PATCH << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_PRE_RELEASE = " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_PRE_RELEASE << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_SNAPSHOT_SN = " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_SNAPSHOT_SN << '\n'
       << setw(40) << "LYRAHGAMES_ROBIN_HOOD_SNAPSHOT_ID = " << setw(40)
       << LYRAHGAMES_ROBIN_HOOD_SNAPSHOT_ID << '\n';
}