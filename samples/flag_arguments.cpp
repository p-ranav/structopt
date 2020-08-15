#include <structopt/app.hpp>

struct Options {
  // verbosity flag
  // -v, --verbose
  // remember to provide a default value
  std::optional<bool> verbose = false;
};
STRUCTOPT(Options, verbose);

int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("my_app").parse<Options>(argc, argv);

    if (options.verbose == true) {
      std::cout << "Verbosity enabled\n";
    }
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
