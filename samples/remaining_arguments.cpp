#include <structopt/app.hpp>

struct CompilerOptions {
  // Language standard
  // e.g., --std c++17
  std::optional<std::string> std;

  // remaining arguments
  // e.g., ./compiler file1 file2 file3
  std::vector<std::string> files{};
};
STRUCTOPT(CompilerOptions, std, files);

int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("my_app").parse<CompilerOptions>(argc, argv);

    std::cout << "Standard : " << options.std.value_or("not provided") << "\n";
    std::cout << "Files    : { ";
    std::copy(options.files.begin(), options.files.end(),
              std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << "}" << std::endl;
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}