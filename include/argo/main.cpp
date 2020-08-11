#include <argo/argo.hpp>
#include <utility>

struct Foo {
  // positional arguments
  int foo{0};
  float bar{0.0f};
  bool baz{false};
  std::array<char, 5> chars;

  // optional "flag" argument
  // -v or --verbose works
  std::optional<bool> verbose;

  // optional "flag" ARRAY argument
  // --blah 1 2 3
  std::optional<std::array<int, 3>> blah;
};
ARGO_COMMAND(Foo, foo, bar, baz, chars, verbose, blah);

int main(int argc, char *argv[]) {
  auto foo = argo::parse<Foo>(argc, argv);
  std::cout << foo.foo << " " << foo.bar << " " << std::boolalpha << foo.baz << "\n";

  for (auto& c : foo.chars) {
    std::cout << c << " ";
  }

  std::cout << "\nVerbose? " << std::boolalpha << foo.verbose.value_or(false) << "\n";

  if (foo.blah.has_value()) {
    auto blah = foo.blah.value();
    for (auto& v : blah) {
      std::cout << v << " ";
    }
    std::cout << "\n";
  }
}