#include <structopt/app.hpp>

struct FloatLiterals {
  std::vector<float> numbers;
};
STRUCTOPT(FloatLiterals, numbers);

int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<FloatLiterals>(argc, argv);

  for (auto &n : options.numbers)
    std::cout << n << "\n";
}
