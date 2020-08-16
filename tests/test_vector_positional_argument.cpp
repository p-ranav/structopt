#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct VectorIntArgument {
  std::vector<int> value = {};
};

STRUCTOPT(VectorIntArgument, value);

TEST_CASE("structopt can parse vector positional argument" * test_suite("vector_positional")) {
  // Vector of ints
  {
    auto arguments = structopt::app("test").parse<VectorIntArgument>(std::vector<std::string>{"./main", "1", "2", "3"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
  }
  {
    auto arguments = structopt::app("test").parse<VectorIntArgument>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.value.empty());
  }
}

struct VectorIntArgumentWithOtherFlags {
  std::vector<int> value = {};
  std::optional<bool> foo = false;
};

STRUCTOPT(VectorIntArgumentWithOtherFlags, value, foo);

TEST_CASE("structopt can parse vector positional argument" * test_suite("vector_positional")) {
  {
    auto arguments = structopt::app("test").parse<VectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "2", "3", "--foo"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<VectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "--foo", "1", "2", "3"});
    REQUIRE(arguments.value == std::vector<int>{1, 2, 3});
    REQUIRE(arguments.foo == true);
  }
  {
    auto arguments = structopt::app("test").parse<VectorIntArgumentWithOtherFlags>(std::vector<std::string>{"./main", "1", "2", "--foo", "3"});
    REQUIRE(arguments.value == std::vector<int>{1, 2});
    REQUIRE(arguments.foo == true);
  }
}

struct VectorOfPairs {
  std::vector<std::pair<std::string, int>> values = {};
};

STRUCTOPT(VectorOfPairs, values);

TEST_CASE("structopt can parse vector of pairs positional argument" * test_suite("vector_positional")) {
  {
    auto arguments = structopt::app("test").parse<VectorOfPairs>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.values == std::vector<std::pair<std::string, int>>{});
  }
  {
    auto arguments = structopt::app("test").parse<VectorOfPairs>(std::vector<std::string>{"./main", "a", "1", "b", "2"});
    REQUIRE(arguments.values == std::vector<std::pair<std::string, int>>{{"a", 1}, {"b", 2}});
  }
  {
    auto arguments = structopt::app("test").parse<VectorOfPairs>(std::vector<std::string>{"./main", "a", "1", "b", "2", "c", "3", "d", "4"});
    REQUIRE(arguments.values == std::vector<std::pair<std::string, int>>{{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}});
  }
  {
    bool exception_thrown = false;
    try {
      auto arguments = structopt::app("test").parse<VectorOfPairs>(std::vector<std::string>{"./main", "a", "1", "b"});
    } catch (structopt::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }  
}

struct CompilerOptionsForVectorOfFiles {
  // Language standard
  // e.g., --std c++17
  std::optional<std::string> std;

  // remaining arguments
  // e.g., ./compiler file1 file2 file3
  std::vector<std::string> files{};
};
STRUCTOPT(CompilerOptionsForVectorOfFiles, std, files);

TEST_CASE("structopt can parse vector of files with an optional argument in the way" * test_suite("vector_positional")) {
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForVectorOfFiles>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.std.has_value() == false);
    REQUIRE(arguments.files == std::vector<std::string>{});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForVectorOfFiles>(std::vector<std::string>{"./main", "file1", "file2", "file3", "-std=c++17"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.files == std::vector<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForVectorOfFiles>(std::vector<std::string>{"./main", "file1", "file2", "file3", "-std:c++17"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.files == std::vector<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForVectorOfFiles>(std::vector<std::string>{"./main", "file1", "file2", "file3", "--std", "c++17"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++17");
    REQUIRE(arguments.files == std::vector<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForVectorOfFiles>(std::vector<std::string>{"./main", "-std=c++20", "file1", "file2", "file3"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++20");
    REQUIRE(arguments.files == std::vector<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForVectorOfFiles>(std::vector<std::string>{"./main", "-std", "c++20", "file1", "file2", "file3"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++20");
    REQUIRE(arguments.files == std::vector<std::string>{"file1", "file2", "file3"});
  }
  {
    auto arguments = structopt::app("test").parse<CompilerOptionsForVectorOfFiles>(std::vector<std::string>{"./main", "-std:c++20"});
    REQUIRE(arguments.std.has_value() == true);
    REQUIRE(arguments.std.value() == "c++20");
    REQUIRE(arguments.files == std::vector<std::string>{});
  }
}
