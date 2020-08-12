#include <doctest.hpp>
#include <structopt/structopt.hpp>

using doctest::test_suite;

// struct Command {  
//   struct SubCommand1 {
//     int bar;
//     struct SubCommand2 {
//       double value;
//       std::optional<bool> verbose;
//     };
//     std::optional<SubCommand2> baz;
//   };
//   std::optional<SubCommand1> foo;
// };
// STRUCTOPT(Command::SubCommand1::SubCommand2, value, verbose);
// STRUCTOPT(Command::SubCommand1, bar, baz);
// STRUCTOPT(Command, foo);

// TEST_CASE("structopt can parse multiple nested struct arguments" * test_suite("nested_struct")) {
//   {
//     auto arguments = structopt::parse<Command>(std::vector<std::string>{"./main", "--foo", "15", "--baz" "3.14", "--verbose", "true"});
//     REQUIRE(arguments.foo.value().bar == 15);
//     REQUIRE(arguments.foo.value().baz.value().value == 3.14);
//     REQUIRE(arguments.foo.value().baz.value().verbose.value() == true);
//   }
// }