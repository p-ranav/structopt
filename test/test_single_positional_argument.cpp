#include <doctest.hpp>
#include <structopt/structopt.hpp>

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

STRUCTOPT(SingleIntArgument, value);
STRUCTOPT(SingleFloatArgument, value);
STRUCTOPT(SingleCharArgument, value);
STRUCTOPT(SingleBoolArgument, value);
STRUCTOPT(SingleStringArgument, value);

TEST_CASE("structopt can parse single positional argument" * test_suite("single_positional")) {
  // Int
  {
    auto arguments = structopt::parse<SingleIntArgument>(std::vector<std::string>{"./main", "5"});
    REQUIRE(arguments.value == 5);
  }
  {
    auto arguments = structopt::parse<SingleIntArgument>(std::vector<std::string>{"./main", "10"});
    REQUIRE(arguments.value == 10);
  }

  // Float
  {
    auto arguments = structopt::parse<SingleFloatArgument>(std::vector<std::string>{"./main", "3.14"});
    REQUIRE(arguments.value == 3.14f);
  }
  {
    auto arguments = structopt::parse<SingleFloatArgument>(std::vector<std::string>{"./main", "10.25893"});
    REQUIRE(arguments.value == 10.25893f);
  }

  // Char
  {
    auto arguments = structopt::parse<SingleCharArgument>(std::vector<std::string>{"./main", "a"});
    REQUIRE(arguments.value == 'a');
  }
  {
    auto arguments = structopt::parse<SingleCharArgument>(std::vector<std::string>{"./main", "%"});
    REQUIRE(arguments.value == '%');
  }

  // Bool
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "true"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "on"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "1"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "yes"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "false"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "off"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "no"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "0"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "TRUE"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "ON"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "YES"});
    REQUIRE(arguments.value == true);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "FALSE"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "OFF"});
    REQUIRE(arguments.value == false);
  }
  {
    auto arguments = structopt::parse<SingleBoolArgument>(std::vector<std::string>{"./main", "NO"});
    REQUIRE(arguments.value == false);
  }

  // String
  {
    auto arguments = structopt::parse<SingleStringArgument>(std::vector<std::string>{"./main", "foobar"});
    REQUIRE(arguments.value == "foobar");
  }
  {
    auto arguments = structopt::parse<SingleStringArgument>(std::vector<std::string>{"./main", "Hello, 世界"});
    REQUIRE(arguments.value == "Hello, 世界");
  }
}