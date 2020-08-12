#include <doctest.hpp>
#include <structopt/structopt.hpp>

using doctest::test_suite;

struct Command {  
  struct SubCommand {
    int bar;
  };
  SubCommand foo;
};
STRUCTOPT(Command::SubCommand, bar);
STRUCTOPT(Command, foo);

TEST_CASE("structopt can parse nested struct arguments" * test_suite("nested_struct")) {
  {
    auto arguments = structopt::parse<Command>(std::vector<std::string>{"./main", "15"});
    REQUIRE(arguments.foo.bar == 15);
  }
}