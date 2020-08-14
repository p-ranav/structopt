#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct TopLevelCommand {
  struct SubCommand : structopt::sub_command {
    int bar;
    std::optional<bool> local;
  };
  SubCommand foo;
};

STRUCTOPT(TopLevelCommand::SubCommand, bar, local);
STRUCTOPT(TopLevelCommand, foo);

TEST_CASE("structopt can parse nested structs" * test_suite("nested_struct")) {
  {
    auto arguments = structopt::app("test").parse<TopLevelCommand>(std::vector<std::string>{"./main", "foo", "15"});
    REQUIRE(arguments.foo.has_value() == true);
    REQUIRE(arguments.foo.bar == 15);
    REQUIRE(arguments.foo.local.has_value() == false);
  }
  {
    auto arguments = structopt::app("test").parse<TopLevelCommand>(std::vector<std::string>{"./main", "foo", "--local", "true", "15"});
    REQUIRE(arguments.foo.has_value() == true);
    REQUIRE(arguments.foo.bar == 15);
    REQUIRE(arguments.foo.local.has_value() == true);
  }
  {
    auto arguments = structopt::app("test").parse<TopLevelCommand>(std::vector<std::string>{"./main", "foo", "15", "--local", "true"});
    REQUIRE(arguments.foo.has_value() == true);
    REQUIRE(arguments.foo.bar == 15);
    REQUIRE(arguments.foo.local.has_value() == true);
  }
}