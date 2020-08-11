#include <argo/argo.hpp>

struct Foo {
  int foo{0};
  float bar{0.0f};
  bool baz{false};
};
ARGO_COMMAND(Foo, foo, bar, baz);

int main(int argc, char *argv[]) {
  auto foo = argo::parse<Foo>(argc, argv);
  std::cout << foo.foo << " " << foo.bar << " " << std::boolalpha << foo.baz << "\n";
}