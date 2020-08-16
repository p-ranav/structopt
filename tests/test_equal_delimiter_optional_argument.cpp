#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct OptionalArgumentFoo {
  std::optional<std::string> foo;
};

STRUCTOPT(OptionalArgumentFoo, foo);

TEST_CASE("structopt can parse optional argument with equal delimited value" * test_suite("single_optional")) {
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFoo>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.foo.has_value() == false);
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFoo>(std::vector<std::string>{"./main", "--foo=bar"});
    REQUIRE(arguments.foo == "bar");
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFoo>(std::vector<std::string>{"./main", "--foo:bar"});
    REQUIRE(arguments.foo == "bar");
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFoo>(std::vector<std::string>{"./main", "--foo:bar=baz"});
    REQUIRE(arguments.foo == "bar=baz");
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFoo>(std::vector<std::string>{"./main", "--foo::bar"});
    REQUIRE(arguments.foo == ":bar");
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFoo>(std::vector<std::string>{"./main", "--foo:=bar"});
    REQUIRE(arguments.foo == "=bar");
  }
}

struct OptionalArgumentFooBar {
  std::optional<std::string> foo_bar;
};

STRUCTOPT(OptionalArgumentFooBar, foo_bar);

TEST_CASE("structopt can parse optional argument with equal delimited value" * test_suite("single_optional")) {
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFooBar>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.foo_bar.has_value() == false);
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFooBar>(std::vector<std::string>{"./main", "--foo-bar=baz"});
    REQUIRE(arguments.foo_bar == "baz");
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFooBar>(std::vector<std::string>{"./main", "--foo_bar:baz"});
    REQUIRE(arguments.foo_bar == "baz");
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFooBar>(std::vector<std::string>{"./main", "-foo-bar:baz=taz"});
    REQUIRE(arguments.foo_bar == "baz=taz");
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFooBar>(std::vector<std::string>{"./main", "-foo_bar::baz"});
    REQUIRE(arguments.foo_bar == ":baz");
  }
  {
    auto arguments = structopt::app("test").parse<OptionalArgumentFooBar>(std::vector<std::string>{"./main", "-f:=baz"});
    REQUIRE(arguments.foo_bar == "=baz");
  }
}