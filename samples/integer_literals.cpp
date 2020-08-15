#include <structopt/app.hpp>

struct IntegerLiterals {
  std::vector<int> numbers;
};
STRUCTOPT(IntegerLiterals, numbers);

int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("my_app").parse<IntegerLiterals>(argc, argv);

    for (auto &n : options.numbers)
      std::cout << n << "\n";
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
