#include <structopt/app.hpp>

struct Options {
  // Flag arguments
  std::optional<bool> a = false;
  std::optional<bool> b = false;

  // Optional argument
  // e.g., -c 1.1 2.2
  std::optional<std::array<float, 2>> c = {};
};
STRUCTOPT(Options, a, b, c);



int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  // Print parsed arguments:

  std::cout << std::boolalpha << "a = " << options.a.value() << ", b = " << options.b.value() << "\n";
  if (options.c.has_value()) {
    std::cout << "c = [" << options.c.value()[0] << ", " << options.c.value()[1] << "]\n";
  }
}