#include <structopt/app.hpp>

struct Options {
  // Positional arguments
  std::array<int, 3> numbers = {0, 0, 0};

  // Flag arguments
  std::optional<bool> a = false;
  std::optional<bool> b = false;

  // Optional argument
  // e.g., -c 1.1 2.2
  std::optional<std::array<float, 2>> c = {};

  // Remaining arguments
  std::optional<std::vector<std::string>> files;
};
STRUCTOPT(Options, numbers, a, b, c, files);



int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  // Print parsed arguments:

  std::cout << "numbers = [" << options.numbers[0] << ", " << options.numbers[1] << ", " << options.numbers[2] << "]\n";
  std::cout << std::boolalpha << "a = " << options.a.value() << ", b = " << options.b.value() << "\n";
  if (options.c.has_value()) {
    std::cout << "c = [" << options.c.value()[0] << ", " << options.c.value()[1] << "]\n";
  }
  if (options.files.has_value()) {
    std::cout << "files = ";
    for (auto& f : options.files.value()) std::cout << f << " ";
    std::cout << "\n";
  }
}