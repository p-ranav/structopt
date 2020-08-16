#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct SingleOptionalFlagArgument {
  std::optional<bool> verbose = false;
};

STRUCTOPT(SingleOptionalFlagArgument, verbose);

TEST_CASE("structopt can parse single optional argument" * test_suite("single_optional")) {
  {
    auto arguments = structopt::app("test").parse<SingleOptionalFlagArgument>(std::vector<std::string>{"./main", "--verbose"});
    REQUIRE(arguments.verbose == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleOptionalFlagArgument>(std::vector<std::string>{"./main", "-verbose"});
    REQUIRE(arguments.verbose == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleOptionalFlagArgument>(std::vector<std::string>{"./main", "-v"});
    REQUIRE(arguments.verbose == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleOptionalFlagArgument>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.verbose == false);
  }
}