#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct StackIntArgument {
  std::stack<int> value = {};
};

STRUCTOPT(StackIntArgument, value);

TEST_CASE("structopt can parse stack positional argument" * test_suite("stack_positional")) {
  // List of ints
  {
    auto arguments = structopt::app("test").parse<StackIntArgument>(std::vector<std::string>{"./main", "1", "2", "3"});
    REQUIRE(arguments.value.size() == 3);
    REQUIRE(arguments.value.top() == 3);
    arguments.value.pop();
    REQUIRE(arguments.value.top() == 2);
    arguments.value.pop();
    REQUIRE(arguments.value.top() == 1);
  }
}

struct QueueIntArgument {
  std::queue<int> value = {};
};

STRUCTOPT(QueueIntArgument, value);

TEST_CASE("structopt can parse queue positional argument" * test_suite("queue_positional")) {
  // List of ints
  {
    auto arguments = structopt::app("test").parse<QueueIntArgument>(std::vector<std::string>{"./main", "1", "2", "3"});
    REQUIRE(arguments.value.size() == 3);
    REQUIRE(arguments.value.front() == 1);
    arguments.value.pop();
    REQUIRE(arguments.value.front() == 2);
    arguments.value.pop();
    REQUIRE(arguments.value.front() == 3);
  }
}

struct PriorityQueueIntArgument {
  // ascending order
  std::priority_queue<int, std::vector<int>, std::greater<int>> value = {};
};

STRUCTOPT(PriorityQueueIntArgument, value);

TEST_CASE("structopt can parse priority_queue positional argument" * test_suite("priority_queue_positional")) {
  // List of ints
  {
    auto arguments = structopt::app("test").parse<PriorityQueueIntArgument>(std::vector<std::string>{"./main", "3", "2", "1"});
    REQUIRE(arguments.value.size() == 3);
    REQUIRE(arguments.value.top() == 1);
    arguments.value.pop();
    REQUIRE(arguments.value.top() == 2);
    arguments.value.pop();
    REQUIRE(arguments.value.top() == 3);
  }
}