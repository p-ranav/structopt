#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct FlagArguments {
  std::optional<bool> a = false;
  std::optional<bool> b = false;
  std::optional<bool> c = false;
};

STRUCTOPT(FlagArguments, a, b, c);

TEST_CASE("structopt can parse combined optional flag arguments" * test_suite("combined_optional")) {
  {
    auto arguments = structopt::app("test").parse<FlagArguments>(std::vector<std::string>{"./main", "-abc"});
    REQUIRE(arguments.a == true);
    REQUIRE(arguments.b == true);
    REQUIRE(arguments.c == true);
  }
  {
    auto arguments = structopt::app("test").parse<FlagArguments>(std::vector<std::string>{"./main", "-a"});
    REQUIRE(arguments.a == true);
    REQUIRE(arguments.b == false);
    REQUIRE(arguments.c == false);
  }
  {
    auto arguments = structopt::app("test").parse<FlagArguments>(std::vector<std::string>{"./main", "-bc"});
    REQUIRE(arguments.a == false);
    REQUIRE(arguments.b == true);
    REQUIRE(arguments.c == true);
  }
  {
    auto arguments = structopt::app("test").parse<FlagArguments>(std::vector<std::string>{"./main", "-ac"});
    REQUIRE(arguments.a == true);
    REQUIRE(arguments.b == false);
    REQUIRE(arguments.c == true);
  }
}