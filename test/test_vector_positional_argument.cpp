#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct VectorIntArgument {
  std::vector<int> value = {};
};

STRUCTOPT(VectorIntArgument, value);

TEST_CASE("structopt can parse vector positional argument" * test_suite("vector_positional")) {
  // Vector of ints
  {
    auto arguments = structopt::app("test").parse<VectorIntArgument>(std::vector<std::string>{"./main", "1", "2", "3"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
  }
}

struct VectorIntArgumentWithOtherFlags {
  std::vector<int> value = {};
  std::optional<bool> foo = false;
};

STRUCTOPT(VectorIntArgumentWithOtherFlags, value, foo);

TEST_CASE("structopt can parse vector positional argument" * test_suite("vector_positional")) {
  {
    auto arguments = structopt::app("test").parse<VectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "2", "3", "--foo"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<VectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--foo", "1", "2", "3"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<VectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "2", "--foo", "3"});
    REQUIRE(arguments.value == std::vector<int>{1, 2});
    REQUIRE(arguments.foo == true);
  }
}