#include <structopt/app.hpp>

struct IntegerLiterals {
  std::vector<int> numbers;
};
STRUCTOPT(IntegerLiterals, numbers);

int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<IntegerLiterals>(argc, argv);

  for (auto &n : options.numbers)
    std::cout << n << "\n";
}
