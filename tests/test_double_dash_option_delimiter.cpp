#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct GrepOptions {
  // reverse the matching
  std::optional<bool> v = false;
  
  // positional arguments
  std::string search;
  std::string pathspec;
};
STRUCTOPT(GrepOptions, v, search, pathspec);

TEST_CASE("structopt can parse the '--' delimiter as end of optional arguments" * test_suite("single_optional")) {
  {
    auto arguments = structopt::app("test").parse<GrepOptions>(std::vector<std::string>{"grep", "--", "-v", "file.csv"});
    REQUIRE(arguments.v == false);
    REQUIRE(arguments.search == "-v");
    REQUIRE(arguments.pathspec == "file.csv");
  }
}

struct CommandOptions {
  struct Sed : structopt::sub_command {
    // --trace
    std::optional<bool> trace = false;

    // remaining args
    std::vector<std::string> args;

    // pattern
    std::string pattern;

    // file
    std::string file;
  };
  Sed sed;
};
STRUCTOPT(CommandOptions::Sed, trace, args, pattern, file);
STRUCTOPT(CommandOptions, sed);

TEST_CASE("structopt can parse the '--' delimiter as end of optional arguments in nested structs" * test_suite("single_optional")) {
  {
    auto arguments = structopt::app("test").parse<CommandOptions>(std::vector<std::string>{"./main", "sed", "--trace", "X=1", "Y=2", "Z=3", "--", "s/foo/bar/g", "foo.txt"});
    REQUIRE(arguments.sed.trace == true);
    REQUIRE(arguments.sed.args == std::vector<std::string>{"X=1", "Y=2", "Z=3"});
    REQUIRE(arguments.sed.pattern == "s/foo/bar/g");
    REQUIRE(arguments.sed.file == "foo.txt");
  }
}