#include <doctest.hpp>
#include <structopt/structopt.hpp>

using doctest::test_suite;

struct Command {
  int foo;
  /*  
  struct SubCommand {
    int bar;
  };
  SubCommand foo;
  */
};
// STRUCTOPT(Command::SubCommand, bar);
STRUCTOPT(Command, foo);

TEST_CASE("structopt can parse nested struct arguments" * test_suite("nested_struct")) {
  {
    // std::vector<std::string> args = {"./main", "15"};
    // auto arguments = structopt::parse<Command>(args);
    // REQUIRE(arguments.foo.bar == 15);
  }
}
