#include <doctest.hpp>
#include <argo/argo.hpp>

using doctest::test_suite;

struct SingleIntArgument {
  int value = 0;
};

struct SingleFloatArgument {
  float value = 0.0f;
};

struct SingleCharArgument {
  char value = '\0';
};

struct SingleBoolArgument {
  bool value = false;
};

struct SingleStringArgument {
  std::string value = "";
};

ARGO_COMMAND(SingleIntArgument, value);
ARGO_COMMAND(SingleFloatArgument, value);
ARGO_COMMAND(SingleCharArgument, value);
ARGO_COMMAND(SingleBoolArgument, value);
ARGO_COMMAND(SingleStringArgument, value);

TEST_CASE("Argo can parse single positional argument" * test_suite("single_positional")) {
  // Int
  {
    auto arguments = argo::parse<SingleIntArgument>(std::vector<std::string>{"./main", "5"});
    REQUIRE(arguments.value == 5);
  }
  {
    auto arguments = argo::parse<SingleIntArgument>(std::vector<std::string>{"./main", "10"});
    REQUIRE(arguments.value == 10);
  }

  // Float
  {
    auto arguments = argo::parse<SingleFloatArgument>(std::vector<std::string>{"./main", "3.14"});
    REQUIRE(arguments.value == 3.14f);
  }
  {
    auto arguments = argo::parse<SingleFloatArgument>(std::vector<std::string>{"./main", "10.25893"});
    REQUIRE(arguments.value == 10.25893f);
  }

  // Char
  {
    auto arguments = argo::parse<SingleCharArgument>(std::vector<std::string>{"./main", "a"});
    REQUIRE(arguments.value == 'a');
  }
  {
    auto arguments = argo::parse<SingleCharArgument>(std::vector<std::string>{"./main", "%"});
    REQUIRE(arguments.value == '%');
  }

  // Bool
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "true"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "on"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "1"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "yes"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "false"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "off"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "no"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "0"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "TRUE"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "ON"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "YES"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "FALSE"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "OFF"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = argo::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "NO"});
    REQUIRE(arguments.value == false);
  }

  // String
  {
    auto arguments = argo::parse<SingleStringArgument>(std::vector<std::string>{"./main", "foobar"});
    REQUIRE(arguments.value == "foobar");
  }
  {
    auto arguments = argo::parse<SingleStringArgument>(std::vector<std::string>{"./main", "Hello, 世界"});
    REQUIRE(arguments.value == "Hello, 世界");
  }
}