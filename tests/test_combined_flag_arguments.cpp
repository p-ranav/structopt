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

struct FlagAndOptionArguments {
  std::optional<bool> a = false;
  std::optional<bool> b = false;
  std::optional<std::array<float, 2>> c = {};
};
STRUCTOPT(FlagAndOptionArguments, a, b, c);

TEST_CASE("structopt can parse combined optional and flag arguments" * test_suite("combined_optional")) {
  {
    auto arguments = structopt::app("test").parse<FlagAndOptionArguments>(std::vector<std::string>{"./main", "-ac", "3.14", "2.718"});
    REQUIRE(arguments.a == true);
    REQUIRE(arguments.b == false);
    REQUIRE(arguments.c == std::array<float, 2>{3.14f, 2.718f});
  }
}