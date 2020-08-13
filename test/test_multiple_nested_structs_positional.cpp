#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct Command {  
  struct SubCommand1 {
    int bar{0};
    double value{0.0};
    std::optional<bool> verbose;
  };
  SubCommand1 foo;

  struct SubCommand2 {
    std::optional<bool> global;
  };
  SubCommand2 config;
};
STRUCTOPT(Command::SubCommand1, bar, value, verbose);
STRUCTOPT(Command::SubCommand2, global);
STRUCTOPT(Command, foo, config);

TEST_CASE("structopt can parse multiple nested struct arguments" * test_suite("nested_struct")) {
  {
    auto arguments = structopt::app("test").parse<Command>(std::vector<std::string>{"./main", "foo", "15", "3.14", "--verbose", "true"});
    REQUIRE(arguments.foo.bar == 15);
    REQUIRE(arguments.foo.value == 3.14);
    REQUIRE(arguments.foo.verbose.value() == true);
    REQUIRE(not arguments.config.global.has_value());
  }
  {
    auto arguments = structopt::app("test").parse<Command>(std::vector<std::string>{"./main", "foo", "-v", "true", "15", "3.14"});
    REQUIRE(arguments.foo.bar == 15);
    REQUIRE(arguments.foo.value == 3.14);
    REQUIRE(arguments.foo.verbose.value() == true);
    REQUIRE(not arguments.config.global.has_value());
  }
  {
    auto arguments = structopt::app("test").parse<Command>(std::vector<std::string>{"./main", "config", "--global", "true"});
    REQUIRE(arguments.foo.bar == 0);
    REQUIRE(arguments.foo.value == 0.0);
    REQUIRE(not arguments.foo.verbose.has_value());
    REQUIRE(arguments.config.global == true);
  }
  {
    auto arguments = structopt::app("test").parse<Command>(std::vector<std::string>{"./main", "config", "-g", "false"});
    REQUIRE(arguments.foo.bar == 0);
    REQUIRE(arguments.foo.value == 0.0);
    REQUIRE(not arguments.foo.verbose.has_value());
    REQUIRE(arguments.config.global == false);
  }
}

struct Git {
  // Subcommand: git config
  struct Config {
    std::optional<bool> global = false;
    std::optional<bool> local  = true;
    std::array<std::string, 2> name_value_pair{};
  };
  Config config;

  // Subcommand: git init
  struct Init {
    std::string name;
  };
  Init init;
};
STRUCTOPT(Git::Config, global, local, name_value_pair);
STRUCTOPT(Git::Init, name);
STRUCTOPT(Git, config, init);

TEST_CASE("structopt can parse multiple nested struct arguments - Git example" * test_suite("nested_struct")) {
  {
    auto arguments = structopt::app("test").parse<Git>(std::vector<std::string>{"./main", "config", "user.name", "Foobar"});
    REQUIRE(arguments.config.global == false);
    REQUIRE(arguments.config.local == true);
    REQUIRE(arguments.config.name_value_pair == std::array<std::string, 2>{"user.name", "Foobar"});
    REQUIRE(arguments.init.name == "");
  }
  {
    auto arguments = structopt::app("test").parse<Git>(std::vector<std::string>{"./main", "config", "--global", "user.name", "Foobar"});
    REQUIRE(arguments.config.global == true);
    REQUIRE(arguments.config.name_value_pair == std::array<std::string, 2>{"user.name", "Foobar"});
    REQUIRE(arguments.init.name == "");
  }
  {
    auto arguments = structopt::app("test").parse<Git>(std::vector<std::string>{"./main", "init", "my_repo"});
    REQUIRE(arguments.config.global == false);
    REQUIRE(arguments.config.name_value_pair == std::array<std::string, 2>{});
    REQUIRE(arguments.init.name == "my_repo");
  }
}