#include <doctest.hpp>
#include <structopt/structopt.hpp>

using doctest::test_suite;

struct SingleOptionalBoolArgument {
  std::optional<bool> verbose;
};

STRUCTOPT(SingleOptionalBoolArgument, verbose);

TEST_CASE("structopt can parse single optional argument" * test_suite("single_optional")) {
  {
    auto arguments = structopt::parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main", "--verbose", "true"});
    REQUIRE(arguments.verbose == true);
  }
  {
    auto arguments = structopt::parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main", "-v", "false"});
    REQUIRE(arguments.verbose == false);
  }
  {
    auto arguments = structopt::parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.verbose.has_value() == false);
  }
}