#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct GccOptions {
  std::optional<std::string> std = "c++11";

  // flag arguments:

  // verbosity enabled with `-v` or `--verbose`
  // or `-verbose`
  std::optional<bool> verbose = false;

  // enable all warnings with `-Wall`
  std::optional<bool> Wall = false;

  // produce only the compiled code
  // e.g., gcc -C main.c
  std::optional<bool> Compile = false;

  // produce output with `-o <exec_name>`
  std::optional<std::string> output = "a.out";

  std::string input_file;
};
STRUCTOPT(GccOptions, std, verbose, Wall, Compile, output, input_file);

TEST_CASE("structopt can parse optional args with delimiters like `=` and `:`" * test_suite("option_delimiter")) {
  {
    auto arguments = structopt::app("test").parse<GccOptions>(std::vector<std::string>{"./gcc", "-std=c++17", "main.cpp"});
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.verbose == false);
    REQUIRE(arguments.Wall == false);
    REQUIRE(arguments.Compile == false);
    REQUIRE(arguments.output == "a.out");
    REQUIRE(arguments.input_file == "main.cpp");
  }
  {
    auto arguments = structopt::app("test").parse<GccOptions>(std::vector<std::string>{"./gcc", "-v", "-Wall", "-std=c++17", "main.cpp", "-o", "main"});
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.verbose == true);
    REQUIRE(arguments.Wall == true);
    REQUIRE(arguments.Compile == false);
    REQUIRE(arguments.output == "main");
    REQUIRE(arguments.input_file == "main.cpp");
  }
  {
    auto arguments = structopt::app("test").parse<GccOptions>(std::vector<std::string>{"./gcc", "main.cpp", "--verbose", "--Wall", "-std:c++17", "--output:main"});
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.verbose == true);
    REQUIRE(arguments.Wall == true);
    REQUIRE(arguments.Compile == false);
    REQUIRE(arguments.output == "main");
    REQUIRE(arguments.input_file == "main.cpp");
  }
  {
    auto arguments = structopt::app("test").parse<GccOptions>(std::vector<std::string>{"./gcc", "main.cpp", "-v", "-W", "-s=c++17", "-o=main"});
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.verbose == true);
    REQUIRE(arguments.Wall == true);
    REQUIRE(arguments.Compile == false);
    REQUIRE(arguments.output == "main");
    REQUIRE(arguments.input_file == "main.cpp");
  }
}