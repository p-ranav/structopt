#include <structopt/app.hpp>

struct Option {
  int input{0};
  std::optional<bool> verbose = false;
};
STRUCTOPT(Option, input, verbose);

int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Option>(argc, argv);

  if (options.verbose == true) {
    std::cout << "The square of " << options.input << " is " << (options.input * options.input) << "\n";
  } else {
    std::cout << options.input * options.input << "\n";
  }
}