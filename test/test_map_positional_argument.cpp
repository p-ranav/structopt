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

struct MapStringMapArgument {
  std::map<std::string, std::map<std::string, int>> value = {};
};

STRUCTOPT(MapStringMapArgument, value);

TEST_CASE("structopt can parse nested map positional argument" * test_suite("map_positional")) {
  // Vector of ints
  {
    auto arguments = structopt::app("test").parse<MapStringMapArgument>(std::vector<std::string>{
        "./main", "a", "b", "2", "c", "5", "d", "6"});
    REQUIRE(arguments.value["a"]["b"] == 2);
    REQUIRE(arguments.value["a"]["c"] == 5);
    REQUIRE(arguments.value["a"]["d"] == 6);
  }
}