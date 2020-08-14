#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct EnumArgument {
  enum class Color {
    red,
    blue,
    green
  };
  Color color;
};
STRUCTOPT(EnumArgument, color);

TEST_CASE("structopt can parse enum class positional argument" * test_suite("enum_positional")) {
  {
    auto arguments = structopt::app("test").parse<EnumArgument>(std::vector<std::string>{"./main", "red"});
    REQUIRE(arguments.color == EnumArgument::Color::red);
  }
  {
    auto arguments = structopt::app("test").parse<EnumArgument>(std::vector<std::string>{"./main", "green"});
    REQUIRE(arguments.color == EnumArgument::Color::green);
  }
  {
    auto arguments = structopt::app("test").parse<EnumArgument>(std::vector<std::string>{"./main", "blue"});
    REQUIRE(arguments.color == EnumArgument::Color::blue);
  }
  {
    bool exception_thrown = false;
    try {
      auto arguments = structopt::app("test").parse<EnumArgument>(std::vector<std::string>{"./main", "black"});
    } catch (std::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }
}

struct EnumArrayArgument {
  enum class Color {
    red,
    blue,
    green
  };
  std::array<Color, 2> colors;
};
STRUCTOPT(EnumArrayArgument, colors);

TEST_CASE("structopt can parse enum array positional argument" * test_suite("enum_positional")) {
  {
    auto arguments = structopt::app("test").parse<EnumArrayArgument>(std::vector<std::string>{"./main", "red", "green"});
    REQUIRE(arguments.colors[0] == EnumArrayArgument::Color::red);
    REQUIRE(arguments.colors[1] == EnumArrayArgument::Color::green);
  }
  {
    auto arguments = structopt::app("test").parse<EnumArrayArgument>(std::vector<std::string>{"./main", "green", "blue"});
    REQUIRE(arguments.colors[0] == EnumArrayArgument::Color::green);
    REQUIRE(arguments.colors[1] == EnumArrayArgument::Color::blue);
  }
  {
    auto arguments = structopt::app("test").parse<EnumArrayArgument>(std::vector<std::string>{"./main", "blue", "red"});
    REQUIRE(arguments.colors[0] == EnumArrayArgument::Color::blue);
    REQUIRE(arguments.colors[1] == EnumArrayArgument::Color::red);
  }
}

struct EnumVectorArgument {
  enum class Color {
    red,
    blue,
    green
  };
  std::vector<Color> colors;
};
STRUCTOPT(EnumVectorArgument, colors);

TEST_CASE("structopt can parse enum vector positional argument" * test_suite("enum_positional")) {
  {
    auto arguments = structopt::app("test").parse<EnumVectorArgument>(std::vector<std::string>{"./main", "red", "green", "blue"});
    REQUIRE(arguments.colors[0] == EnumVectorArgument::Color::red);
    REQUIRE(arguments.colors[1] == EnumVectorArgument::Color::green);
    REQUIRE(arguments.colors[2] == EnumVectorArgument::Color::blue);
  }
  {
    auto arguments = structopt::app("test").parse<EnumVectorArgument>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.colors.empty());
  }
  {
    auto arguments = structopt::app("test").parse<EnumVectorArgument>(std::vector<std::string>{"./main", "blue", "red"});
    REQUIRE(arguments.colors[0] == EnumVectorArgument::Color::blue);
    REQUIRE(arguments.colors[1] == EnumVectorArgument::Color::red);
  }
}

struct EnumPairArgument {
  enum class Color {
    red,
    blue,
    green
  };
  std::pair<Color, Color> colors;
};
STRUCTOPT(EnumPairArgument, colors);

TEST_CASE("structopt can parse enum pair positional argument" * test_suite("enum_positional")) {
  {
    auto arguments = structopt::app("test").parse<EnumPairArgument>(std::vector<std::string>{"./main", "red", "green"});
    REQUIRE(arguments.colors.first == EnumPairArgument::Color::red);
    REQUIRE(arguments.colors.second == EnumPairArgument::Color::green);
  }
}