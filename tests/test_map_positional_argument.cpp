#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct MapStringIntArgument {
  std::map<std::string, int> value = {};
};

STRUCTOPT(MapStringIntArgument, value);

TEST_CASE("structopt can parse map positional argument" * test_suite("map_positional")) {
  // Vector of ints
  {
    auto arguments = structopt::app("test").parse<MapStringIntArgument>(std::vector<std::string>{"./main", "a", "2", "b", "5"});
    REQUIRE(arguments.value["a"] == 2);
    REQUIRE(arguments.value["b"] == 5);
  }
}