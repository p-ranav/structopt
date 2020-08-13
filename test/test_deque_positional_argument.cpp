#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct DequeIntArgument {
  std::deque<int> value = {};
};

STRUCTOPT(DequeIntArgument, value);

TEST_CASE("structopt can parse deque positional argument" * test_suite("vector_positional")) {
  // Deque of ints
  {
    auto arguments = structopt::app("test").parse<DequeIntArgument>(std::vector<std::string>{"./main", "1", "2", "3"});
    REQUIRE(arguments.value == std::deque<int>{1, 2, 3});
  }
}

struct DequeIntArgumentWithOtherFlags {
  std::deque<int> value = {};
  std::optional<bool> foo = false;
};

STRUCTOPT(DequeIntArgumentWithOtherFlags, value, foo);

TEST_CASE("structopt can parse deque positional argument" * test_suite("vector_positional")) {
  {
    auto arguments = structopt::app("test").parse<DequeIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "2", "3", "--foo"});
    REQUIRE(arguments.value == std::deque<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<DequeIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--foo", "1", "2", "3"});
    REQUIRE(arguments.value == std::deque<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<DequeIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "2", "--foo", "3"});
    REQUIRE(arguments.value == std::deque<int>{1, 2});
    REQUIRE(arguments.foo == true);
  }
}