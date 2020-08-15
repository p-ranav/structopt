#include <structopt/app.hpp>

struct FileOptions {
  // Positional arguments
  // ./main <input_file> <output_file>
  std::string input_file;
  std::string output_file;
};
STRUCTOPT(FileOptions, input_file, output_file);

int main(int argc, char *argv[]) {

  try {
    auto options = structopt::app("my_app").parse<FileOptions>(argc, argv);

    // Print parsed arguments:
    std::cout << "\nInput file  : " << options.input_file << "\n";
    std::cout << "Output file : " << options.output_file << "\n";

  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}