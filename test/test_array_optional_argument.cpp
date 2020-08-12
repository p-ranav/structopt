#include <doctest.hpp>
#include <argo/argo.hpp>

using doctest::test_suite;

struct ArrayOptionalArgument {
  std::optional<std::array<int, 3>> indices;
};

ARGO_STRUCT(ArrayOptionalArgument, indices);

TEST_CASE("Argo can parse array optional argument" * test_suite("array_optional")) {
  {
    auto arguments = argo::parse<ArrayOptionalArgument>(std::vector<std::string>{"./main", "--indices", "1", "2", "3"});
    REQUIRE(arguments.indices == std::array<int, 3>{1, 2, 3});
  }
  {
    auto arguments = argo::parse<ArrayOptionalArgument>(std::vector<std::string>{"./main", "-i", "4", "5", "6"});
    REQUIRE(arguments.indices == std::array<int, 3>{4, 5, 6});
  }
  {
    auto arguments = argo::parse<ArrayOptionalArgument>(std::vector<std::string>{"./main"});
    REQUIRE(arguments.indices.has_value() == false);
  }
}