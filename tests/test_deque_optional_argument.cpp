#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct OptionalDequeIntArgument {
  std::optional<std::deque<int>> value = {};
};

STRUCTOPT(OptionalDequeIntArgument, value);

TEST_CASE("structopt can parse deque optional argument" * test_suite("deque_optional")) {
  // Deque of ints
  {
    auto arguments = structopt::app("test").parse<OptionalDequeIntArgument>(std::vector<std::string>{"./main", "--value", "1", "2", "3"});
    REQUIRE(arguments.value.has_value() == true);
    REQUIRE(arguments.value.value() == std::deque<int>{1, 2, 3});
  }
}

struct OptionalDequeIntArgumentWithOtherFlags {
  std::optional<std::deque<int>> value = {};
  std::optional<bool> foo = false;
};

STRUCTOPT(OptionalDequeIntArgumentWithOtherFlags, value, foo);

TEST_CASE("structopt can parse deque optional argument" * test_suite("deque_optional")) {
  {
    auto arguments = structopt::app("test").parse<OptionalDequeIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--value", "1", "2", "3", "--foo"});
    REQUIRE(arguments.value == std::deque<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<OptionalDequeIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--foo", "--value", "1", "2", "3"});
    REQUIRE(arguments.value == std::deque<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    bool exception_thrown{false};
    try {
      auto arguments = structopt::app("test").parse<OptionalDequeIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--value", "1", "2", "--foo", "3"});
      REQUIRE(arguments.value == std::deque<int>{1, 2});
      REQUIRE(arguments.foo == true);
    } catch(structopt::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }
}