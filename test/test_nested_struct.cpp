#include <doctest.hpp>
#include <structopt/structopt.hpp>

using doctest::test_suite;

struct TopLevelCommand {
  struct SubCommand {
    int bar;
    std::optional<bool> local;
  };
  SubCommand foo;
};

STRUCTOPT(TopLevelCommand::SubCommand, bar, local);
STRUCTOPT(TopLevelCommand, foo);

TEST_CASE("structopt can parse nested structs" * test_suite("nested_struct")) {
  {
    auto arguments = structopt::parse<TopLevelCommand>(std::vector<std::string>{"./main", "foo", "15"});
    REQUIRE(arguments.foo.bar == 15);
    REQUIRE(arguments.foo.local.has_value() == false);
  }
  {
    auto arguments = structopt::parse<TopLevelCommand>(std::vector<std::string>{"./main", "foo", "--local", "true", "15"});
    REQUIRE(arguments.foo.bar == 15);
    REQUIRE(arguments.foo.local.has_value() == true);
  }
  {
    auto arguments = structopt::parse<TopLevelCommand>(std::vector<std::string>{"./main", "foo", "15", "--local", "true"});
    REQUIRE(arguments.foo.bar == 15);
    REQUIRE(arguments.foo.local.has_value() == true);
  }
}