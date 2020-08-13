#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct ArrayOptionalArgument {
  std::optional<std::array<int, 3>> indices;
};

STRUCTOPT(ArrayOptionalArgument, indices);

TEST_CASE("structopt can parse array optional argument" * test_suite("array_optional")) {
  {
    auto arguments = structopt::app("test").parse<ArrayOptionalArgument>(std::vector<std::string>{"./main", "--indices", "1", "2", "3"});
    REQUIRE(arguments.indices == std::array<int, 3>{1, 2, 3});
  }
  {
    auto arguments = structopt::app("test").parse<ArrayOptionalArgument>(std::vector<std::string>{"./main", "-i", "4", "5", "6"});
    REQUIRE(arguments.indices == std::array<int, 3>{4, 5, 6});
  }
  {
    auto arguments = structopt::app("test").parse<ArrayOptionalArgument>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.indices.has_value() == false);
  }
  {
    auto arguments = structopt::app("test").parse<ArrayOptionalArgument>(std::vector<std::string>{"./main", "--indices", "-1", "-2", "-3"});
    REQUIRE(arguments.indices == std::array<int, 3>{-1, -2, -3});
  }
  {
    auto arguments = structopt::app("test").parse<ArrayOptionalArgument>(std::vector<std::string>{"./main", "-i", "0x53", "071", "0b01101111"});
    REQUIRE(arguments.indices == std::array<int, 3>{0x53, 071, 0b01101111});
  }
}