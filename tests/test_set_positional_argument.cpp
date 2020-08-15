#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct SetIntArgument {
  std::set<int> value = {};
};
STRUCTOPT(SetIntArgument, value);

TEST_CASE("structopt can parse set positional argument" * test_suite("set_positional")) {
  // Set of ints
  {
    auto arguments = structopt::app("test").parse<SetIntArgument>(std::vector<std::string>{"./main", "1", "2", "3", "1", "4"});
    REQUIRE(arguments.value == std::set<int>{1, 2, 3, 4});
  }
}

struct SetIntArgumentWithOtherFlags {
  std::set<int> value = {};
  std::optional<bool> foo = false;
};
STRUCTOPT(SetIntArgumentWithOtherFlags, value, foo);

TEST_CASE("structopt can parse set positional argument" * test_suite("set_positional")) {
  {
    auto arguments = structopt::app("test").parse<SetIntArgumentWithOtherFlags>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.value.empty());
    REQUIRE(arguments.foo == false);
  }
  {
    auto arguments = structopt::app("test").parse<SetIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "2", "3", "4", "5", "3", "--foo"});
    REQUIRE(arguments.value == std::set<int>{1, 2, 3, 4, 5});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<SetIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--foo", "1", "2", "3", "4"});
    REQUIRE(arguments.value == std::set<int>{1, 2, 3, 4});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<SetIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "1", "--foo", "3"});
    REQUIRE(arguments.value == std::set<int>{1});
    REQUIRE(arguments.foo == true);
  }
}