#include <structopt/structopt.hpp>
#include <doctest.hpp>

using doctest::test_suite;

struct Array3dArgument {
  std::array<std::array<std::array<double,3>,4>, 4> value = {};
};
STRUCTOPT(Array3dArgument, value);

TEST_CASE("structopt can parse fixed 3D array positional argument" *
          test_suite("3d_array_positional")) {
  // 3D array
  {
    auto arguments = structopt::parse<Array3dArgument>(std::vector<std::string>{
        "./main", "0", "0.51", "0.35", "0", "0.51", "0.35", "0", "0.51",
        "0.35",   "0", "0.51", "0.35", "0", "0.51", "0.35", "0", "0.51",
        "0.35",   "0", "0.51", "0.35", "0", "0.51", "0.35", "0", "0.51",
        "0.35",   "0", "0.51", "0.35", "0", "0.51", "0.35", "0", "0.51",
        "0.35",   "0", "0.51", "0.35", "0", "0.51", "0.35", "0", "0.51",
        "0.35",   "0", "0.51", "0.35"});

    REQUIRE(arguments.value ==
            std::array<std::array<std::array<double,3>,4>, 4>{
                {{{{0, 0.51, 0.35},
                   {0, 0.51, 0.35},
                   {0, 0.51, 0.35},
                   {0, 0.51, 0.35}}},
                 {{{0, 0.51, 0.35},
                   {0, 0.51, 0.35},
                   {0, 0.51, 0.35},
                   {0, 0.51, 0.35}}},
                 {{{0, 0.51, 0.35},
                   {0, 0.51, 0.35},
                   {0, 0.51, 0.35},
                   {0, 0.51, 0.35}}},
                 {{{0, 0.51, 0.35},
                   {0, 0.51, 0.35},
                   {0, 0.51, 0.35},
                   {0, 0.51, 0.35}}}}});
  }
}