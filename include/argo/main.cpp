#include <argo/argo.hpp>

struct Foo {
  int foo{0};
  int bar{0};
};
ARGO_COMMAND(Foo, foo, bar);

int main(int argc, char *argv[]) {
  auto foo = argo::parse<Foo>(argc, argv);
  std::cout << foo.foo << " " << foo.bar << "\n";
}