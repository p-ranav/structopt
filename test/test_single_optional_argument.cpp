#include <doctest.hpp>
#include <argo/argo.hpp>

using doctest::test_suite;

struct SingleOptionalBoolArgument {
  std::optional<bool> verbose;
};

ARGO_STRUCT(SingleOptionalBoolArgument, verbose);

TEST_CASE("Argo can parse single optional argument" * test_suite("single_optional")) {
  {
    auto arguments = argo::parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main", "--verbose", "true"});
    REQUIRE(arguments.verbose == true);
  }
  {
    auto arguments = argo::parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main", "-v", "false"});
    REQUIRE(arguments.verbose == false);
  }
  {
    auto arguments = argo::parse<SingleOptionalBoolArgument>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.verbose.has_value() == false);
  }
}