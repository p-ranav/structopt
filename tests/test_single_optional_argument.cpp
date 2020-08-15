#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct SingleOptionalBoolArgument {
  std::optional<bool> verbose;
};

STRUCTOPT(SingleOptionalBoolArgument, verbose);

TEST_CASE("structopt can parse single optional argument" * test_suite("single_optional")) {
  {
    bool exception_thrown = false;
    try {
      auto arguments = structopt::app("test").parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main", "--verbose"});
    } catch (structopt::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main", "--verbose", "true"});
    REQUIRE(arguments.verbose == true);
  }
  {
    bool exception_thrown = false;
    try {
      auto arguments = structopt::app("test").parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main", "--verbose", "trueee"});
    } catch (structopt::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main", "-v", "false"});
    REQUIRE(arguments.verbose == false);
  }
  {
    auto arguments = structopt::app("test").parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.verbose.has_value() == false);
  }
}