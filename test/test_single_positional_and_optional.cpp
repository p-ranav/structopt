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

struct PositionalAndOptionalArrayArgument {
  std::array<int, 2> foo = {};
  std::optional<std::array<float, 3>> bar;
};

ARGO_STRUCT(PositionalAndOptionalArrayArgument, foo, bar);

TEST_CASE("Argo can parse single positional and optional array arguments" * test_suite("single_positional_and_optional")) {
  {
    auto arguments = argo::parse<PositionalAndOptionalArrayArgument>(std::vector<std::string>{"./main", "1", "2"});
    REQUIRE(arguments.foo == std::array<int, 2>{1, 2});
    REQUIRE(not arguments.bar.has_value());
  }
  {
    auto arguments = argo::parse<PositionalAndOptionalArrayArgument>(std::vector<std::string>{"./main", "1", "5", "--bar", "1.1", "2.2", "3.3"});
    REQUIRE(arguments.foo == std::array<int, 2>{1, 5});
    REQUIRE(arguments.bar.value() == std::array<float, 3>{1.1, 2.2, 3.3});
  }
  {
    auto arguments = argo::parse<PositionalAndOptionalArrayArgument>(std::vector<std::string>{"./main", "1", "5", "-b", "1.1", "2.2", "3.3"});
    REQUIRE(arguments.foo == std::array<int, 2>{1, 5});
    REQUIRE(arguments.bar.value() == std::array<float, 3>{1.1, 2.2, 3.3});
  }
  {
    auto arguments = argo::parse<PositionalAndOptionalArrayArgument>(std::vector<std::string>{"./main", "--bar", "1.1", "2.2", "3.3", "1", "5"});
    REQUIRE(arguments.foo == std::array<int, 2>{1, 5});
    REQUIRE(arguments.bar.value() == std::array<float, 3>{1.1, 2.2, 3.3});
  }
  {
    auto arguments = argo::parse<PositionalAndOptionalArrayArgument>(std::vector<std::string>{"./main", "-b", "1.1", "2.2", "3.3", "1", "5"});
    REQUIRE(arguments.foo == std::array<int, 2>{1, 5});
    REQUIRE(arguments.bar.value() == std::array<float, 3>{1.1, 2.2, 3.3});
  }
}