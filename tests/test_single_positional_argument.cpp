#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct SingleIntArgument {
  int value = 0;
};

struct SingleNegativeIntArgument {
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

struct SinglePairArgument {
  std::pair<int, float> value = {0, 0.0f};
};

STRUCTOPT(SingleIntArgument, value);
STRUCTOPT(SingleNegativeIntArgument, value);
STRUCTOPT(SingleFloatArgument, value);
STRUCTOPT(SingleCharArgument, value);
STRUCTOPT(SingleBoolArgument, value);
STRUCTOPT(SingleStringArgument, value);
STRUCTOPT(SinglePairArgument, value);

TEST_CASE("structopt can parse single positional argument" * test_suite("single_positional")) {
  // Int
  {
    auto arguments = structopt::app("test").parse<SingleIntArgument>(std::vector<std::string>{"./main", "5"});
    REQUIRE(arguments.value == 5);
  }
  {
    auto arguments = structopt::app("test").parse<SingleIntArgument>(std::vector<std::string>{"./main", "10"});
    REQUIRE(arguments.value == 10);
  }
  {
    auto arguments = structopt::app("test").parse<SingleIntArgument>(std::vector<std::string>{"./main", "0x5B"});
    REQUIRE(arguments.value == 0x5B);
  }
  {
    auto arguments = structopt::app("test").parse<SingleIntArgument>(std::vector<std::string>{"./main", "051"});
    REQUIRE(arguments.value == 051);
  }
  {
    auto arguments = structopt::app("test").parse<SingleIntArgument>(std::vector<std::string>{"./main", "0b0101"});
    REQUIRE(arguments.value == 0b0101);
  }

  // Negative Int
  {
    auto arguments = structopt::app("test").parse<SingleNegativeIntArgument>(std::vector<std::string>{"./main", "-5"});
    REQUIRE(arguments.value == -5);
  }
  {
    auto arguments = structopt::app("test").parse<SingleNegativeIntArgument>(std::vector<std::string>{"./main", "+10"});
    REQUIRE(arguments.value == 10);
  }

  // Float
  {
    auto arguments = structopt::app("test").parse<SingleFloatArgument>(std::vector<std::string>{"./main", "3.14"});
    REQUIRE(arguments.value == 3.14f);
  }
  {
    auto arguments = structopt::app("test").parse<SingleFloatArgument>(std::vector<std::string>{"./main", "10.25893"});
    REQUIRE(arguments.value == 10.25893f);
  }

  // Char
  {
    auto arguments = structopt::app("test").parse<SingleCharArgument>(std::vector<std::string>{"./main", "a"});
    REQUIRE(arguments.value == 'a');
  }
  {
    auto arguments = structopt::app("test").parse<SingleCharArgument>(std::vector<std::string>{"./main", "%"});
    REQUIRE(arguments.value == '%');
  }

  // Bool
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "true"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "on"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "1"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "yes"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "false"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "off"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "no"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "0"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "TRUE"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "ON"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "YES"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "FALSE"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "OFF"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::app("test").parse<SingleBoolArgument>(std::vector<std::string>{"./main", "NO"});
    REQUIRE(arguments.value == false);
  }

  // String
  {
    auto arguments = structopt::app("test").parse<SingleStringArgument>(std::vector<std::string>{"./main", "foobar"});
    REQUIRE(arguments.value == "foobar");
  }
  {
    auto arguments = structopt::app("test").parse<SingleStringArgument>(std::vector<std::string>{"./main", "Hello, 世界"});
    REQUIRE(arguments.value == "Hello, 世界");
  }
  
  // Pair
  {
    auto arguments = structopt::app("test").parse<SinglePairArgument>(std::vector<std::string>{"./main", "10", "3.14"});
    REQUIRE(arguments.value == std::pair<int, float>{10, 3.14});
  }
}