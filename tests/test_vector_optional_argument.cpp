#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct OptionalVectorIntArgument {
  std::optional<std::vector<int>> value = {};
};

STRUCTOPT(OptionalVectorIntArgument, value);

TEST_CASE("structopt can parse vector optional argument" * test_suite("vector_optional")) {
  // Vector of ints
  {
    auto arguments = structopt::app("test").parse<OptionalVectorIntArgument>(std::vector<std::string>{"./main", "--value", "1", "2", "3"});
    REQUIRE(arguments.value.has_value() == true);
    REQUIRE(arguments.value.value() == std::vector<int>{1, 2, 3});
  }
}

struct OptionalVectorIntArgumentWithOtherFlags {
  std::optional<std::vector<int>> value = {};
  std::optional<bool> foo = false;
};

STRUCTOPT(OptionalVectorIntArgumentWithOtherFlags, value, foo);

TEST_CASE("structopt can parse vector optional argument" * test_suite("vector_optional")) {
  {
    auto arguments = structopt::app("test").parse<OptionalVectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--value", "1", "2", "3", "--foo"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<OptionalVectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--foo", "--value", "1", "2", "3"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    bool exception_thrown{false};
    try {
      auto arguments = structopt::app("test").parse<OptionalVectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--value", "1", "2", "--foo", "3"});
      REQUIRE(arguments.value == std::vector<int>{1, 2});
      REQUIRE(arguments.foo == true);
    } catch(structopt::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }
}