#include <doctest.hpp>
#include <structopt/structopt.hpp>

using doctest::test_suite;

struct GrepOptions {
  // reverse the matching
  std::optional<bool> v;
  
  // positional arguments
  std::string search;
  std::string pathspec;
};
STRUCTOPT(GrepOptions, v, search, pathspec);

TEST_CASE("structopt can parse the '--' delimiter as end of optional arguments" * test_suite("single_optional")) {
  {
    auto arguments = structopt::parse<GrepOptions>(std::vector<std::string>{"grep", "--", "-v", "file.csv"});
    REQUIRE(arguments.v.has_value() == false);
    REQUIRE(arguments.search == "-v");
    REQUIRE(arguments.pathspec == "file.csv");
  }
}