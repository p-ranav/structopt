#include <structopt/app.hpp>

struct Options {
  // positional arguments
  std::string input_file;
  std::string output_file;

  // optional arguments
  std::optional<std::string> bind_address;

  // remaining arguments
  std::vector<std::string> files;
};
STRUCTOPT(Options, input_file, output_file, bind_address, files);

int main(int argc, char *argv[]) {
  auto app = structopt::app("my_app", "1.0.3");

  try {
    auto options = app.parse<Options>(argc, argv);
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
