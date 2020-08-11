#include <doctest.hpp>
#include <argo/argo.hpp>

using doctest::test_suite;

struct ArrayIntArgument {
  std::array<int, 3> value = {};
};

struct ArrayFloatArgument {
  std::array<float, 5> value = {};
};

struct ArrayCharArgument {
  std::array<char, 2> value = {};
};

struct ArrayBoolArgument {
  std::array<bool, 8> value = {};
};

struct ArrayStringArgument {
  std::array<std::string, 4> value = {};
};

ARGO_STRUCT(ArrayIntArgument, value);
ARGO_STRUCT(ArrayFloatArgument, value);
ARGO_STRUCT(ArrayCharArgument, value);
ARGO_STRUCT(ArrayBoolArgument, value);
ARGO_STRUCT(ArrayStringArgument, value);

TEST_CASE("Argo can parse fixed array positional argument" * test_suite("array_positional")) {
  // Array of ints
  {
    auto arguments = argo::parse<ArrayIntArgument>(std::vector<std::string>{"./main", "1", "2", "3"});
    REQUIRE(arguments.value == std::array<int, 3>{1, 2, 3});
  }

  // Array of floats
  {
    auto arguments = argo::parse<ArrayFloatArgument>(std::vector<std::string>{"./main", "1.1", "2.2", "3.3", "4.4", "5.5"});
    REQUIRE(arguments.value == std::array<float, 5>{1.1f, 2.2f, 3.3f, 4.4f, 5.5f});
  }

  // Array of chars
  {
    auto arguments = argo::parse<ArrayCharArgument>(std::vector<std::string>{"./main", "a", "b"});
    REQUIRE(arguments.value == std::array<char, 2>{'a', 'b'});
  }

  // Array of bools
  {
    auto arguments = argo::parse<ArrayBoolArgument>(std::vector<std::string>{"./main", "true", "false", "on", "off", "yes", "no", "1", "0"});
    REQUIRE(arguments.value == std::array<bool, 8>{true, false, true, false, true, false, true, false});
  }

  // Array of strings
  {
    auto arguments = argo::parse<ArrayStringArgument>(std::vector<std::string>{"./main", "abc", "def", "ghi", "jkl"});
    REQUIRE(arguments.value == std::array<std::string, 4>{"abc", "def", "ghi", "jkl"});
  }
}