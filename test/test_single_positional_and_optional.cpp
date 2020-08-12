#include <doctest.hpp>
#include <argo/argo.hpp>

using doctest::test_suite;

struct PositionalAndOptionalArgument {
  int foo = 0;
  std::optional<bool> bar;
};

ARGO_STRUCT(PositionalAndOptionalArgument, foo, bar);

TEST_CASE("Argo can parse single positional and optional arguments" * test_suite("single_positional_and_optional")) {
  {
    auto arguments = argo::parse<PositionalAndOptionalArgument>(std::vector<std::string>{"./main", "19"});
    REQUIRE(arguments.foo == 19);
    REQUIRE(not arguments.bar.has_value());
  }
  {
    auto arguments = argo::parse<PositionalAndOptionalArgument>(std::vector<std::string>{"./main", "91", "--bar", "true"});
    REQUIRE(arguments.foo == 91);
    REQUIRE(arguments.bar == true);
  }
  {
    auto arguments = argo::parse<PositionalAndOptionalArgument>(std::vector<std::string>{"./main", "153", "-b", "true"});
    REQUIRE(arguments.foo == 153);
    REQUIRE(arguments.bar == true);
  }
  {
    auto arguments = argo::parse<PositionalAndOptionalArgument>(std::vector<std::string>{"./main", "--bar", "true", "91"});
    REQUIRE(arguments.foo == 91);
    REQUIRE(arguments.bar == true);
  }
  {
    auto arguments = argo::parse<PositionalAndOptionalArgument>(std::vector<std::string>{"./main", "-b", "true", "153"});
    REQUIRE(arguments.foo == 153);
    REQUIRE(arguments.bar == true);
  }
}