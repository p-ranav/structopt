#include <structopt/app.hpp>

struct FileOptions {
  // Positional arguments
  // ./main <input_file> <output_file>
  std::string input_file;
  std::string output_file;
};
STRUCTOPT(FileOptions, input_file, output_file);



int main(int argc, char *argv[]) {
  auto app = structopt::app("my_app");
  
  try {
    auto options = app.parse<FileOptions>(argc, argv);

    // Print parsed arguments:
    std::cout << "\nInput file  : " << options.input_file << "\n";
    std::cout << "Output file : " << options.output_file << "\n";

  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    app.print_help();
  }
}