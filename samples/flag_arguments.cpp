#include <structopt/app.hpp>

struct Options {
  // verbosity flag
  // -v, --verbose
  // remember to provide a default value
  std::optional<bool> verbose = false;
};
STRUCTOPT(Options, verbose);

int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  if (options.verbose == true) {
    std::cout << "Verbosity enabled\n";
  }
}
