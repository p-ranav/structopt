#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct SetIntArgument {
  std::set<int> value = {};
};
STRUCTOPT(SetIntArgument, value);

TEST_CASE("structopt can parse set positional argument" * test_suite("set_positional")) {
  // Set of ints
  {
    auto arguments = structopt::app("test").parse<SetIntArgument>(std::vector<std::string>{"./main", "1", "2", "3", "1", "4"});
    REQUIRE(arguments.value == std::set<int>{1, 2, 3, 4});
  }
}

struct SetIntArgumentWithOtherFlags {
  std::set<int> value = {};
  std::optional<bool> foo = false;
};
STRUCTOPT(SetIntArgumentWithOtherFlags, value, foo);

TEST_CASE("structopt can parse set positional argument" * test_suite("set_positional")) {
  {
    auto arguments = structopt::app("test").parse<SetIntArgumentWithOtherFlags>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.value.empty());
    REQUIRE(arguments.foo == false);
  }
  {
    auto arguments = structopt::app("test").parse<SetIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "2", "3", "4", "5", "3", "--foo"});
    REQUIRE(arguments.value == std::set<int>{1, 2, 3, 4, 5});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<SetIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--foo", "1", "2", "3", "4"});
    REQUIRE(arguments.value == std::set<int>{1, 2, 3, 4});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<SetIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "1", "--foo", "3"});
    REQUIRE(arguments.value == std::set<int>{1});
    REQUIRE(arguments.foo == true);
  }
}

struct CompilerOptionsForSetOfFiles {
  // Language standard
  // e.g., --std c++17
  std::optional<std::string> std;

  // remaining arguments
  // e.g., ./compiler file1 file2 file3
  std::set<std::string> files{};
};
STRUCTOPT(CompilerOptionsForSetOfFiles, std, files);

TEST_CASE("structopt can parse set of files with an optional argument in the way" * test_suite("vector_positional")) {
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForSetOfFiles>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.std.has_value() == false);
    REQUIRE(arguments.files == std::set<std::string>{});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForSetOfFiles>(std::vector<std::string>{"./main", "file1", "file2", "file3", "-std=c++17"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.files == std::set<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForSetOfFiles>(std::vector<std::string>{"./main", "file1", "file2", "file3", "-std:c++17"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.files == std::set<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForSetOfFiles>(std::vector<std::string>{"./main", "file1", "file2", "file3", "--std", "c++17"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.files == std::set<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForSetOfFiles>(std::vector<std::string>{"./main", "-std=c++20", "file1", "file2", "file3"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++20");
    REQUIRE(arguments.files == std::set<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForSetOfFiles>(std::vector<std::string>{"./main", "-std", "c++20", "file1", "file2", "file3"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++20");
    REQUIRE(arguments.files == std::set<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForSetOfFiles>(std::vector<std::string>{"./main", "-std:c++20"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++20");
    REQUIRE(arguments.files == std::set<std::string>{});
  }
}