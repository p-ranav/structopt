#include <structopt/app.hpp>

struct GccOptions {
  std::optional<std::string> std = "c++11";

  // flag arguments:

  // verbosity enabled with `-v` or `--verbose`
  // or `-verbose`
  std::optional<bool> verbose = false;

  // enable all warnings with `-Wall`
  std::optional<bool> Wall = false;

  // produce only the compiled code
  // e.g., gcc -C main.c
  std::optional<bool> Compile = false;

  // produce output with `-o <exec_name>`
  std::optional<std::string> output = "a.out";

  std::string input_file;
};
STRUCTOPT(GccOptions, std, verbose, Wall, Compile, output, input_file);

int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("gcc").parse<GccOptions>(argc, argv);  

    // Print parsed arguments

    std::cout << "std        : " << options.std.value() << "\n";
    std::cout << "verbose    : " << std::boolalpha << options.verbose.value() << "\n";
    std::cout << "Wall       : " << std::boolalpha << options.Wall.value() << "\n";
    std::cout << "Compile    : " << std::boolalpha << options.Compile.value() << "\n";
    std::cout << "Output     : " << options.output.value() << "\n";
    std::cout << "Input file : " << options.input_file << "\n";
  }
  catch (structopt::exception& e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}