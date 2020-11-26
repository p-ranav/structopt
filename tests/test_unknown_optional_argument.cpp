#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct OptionalArgsOptions {
  std::optional<std::string> directory;
};
STRUCTOPT(OptionalArgsOptions, directory);

TEST_CASE("structopt reports error on unknown optional arguments" * test_suite("single_optional")) {
  {
    bool exception_thrown{false};
    try {
      auto arguments = structopt::app("test").parse<OptionalArgsOptions>(std::vector<std::string>{"./main", "--bad"});
    } catch(structopt::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }
}

TEST_CASE("structopt reports error on unknown positional arguments" * test_suite("single_optional")) {
  {
    bool exception_thrown{false};
    try {
      auto arguments = structopt::app("test").parse<OptionalArgsOptions>(std::vector<std::string>{"./main", "bad"});
    } catch(structopt::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }
}

TEST_CASE("structopt correctly parses optional arguments" * test_suite("single_optional")) {
  {
    auto arguments = structopt::app("test").parse<OptionalArgsOptions>(std::vector<std::string>{"./main", "--directory", "foo/bar"});
    REQUIRE(arguments.directory == "foo/bar");
  }
}