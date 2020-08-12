#include <doctest.hpp>
#include <structopt/structopt.hpp>

using doctest::test_suite;

struct VectorIntArgument {
  std::vector<int> value = {};
};

STRUCTOPT(VectorIntArgument, value);

TEST_CASE("structopt can parse vector positional argument" * test_suite("vector_positional")) {
  // Vector of ints
  {
    auto arguments = structopt::parse<VectorIntArgument>(std::vector<std::string>{"./main", "1", "2", "3"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
  }
}