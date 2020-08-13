#include <structopt/app.hpp>

struct Option {
  std::string input_file = "";
  std::string output_file = "";
};
STRUCTOPT(Option, input_file, output_file);

int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Option>(argc, argv);

  // Use arguments
  std::cout << "Input file: " << options.input_file << "\n";
  std::cout << "Output file: " << options.output_file << "\n";
}