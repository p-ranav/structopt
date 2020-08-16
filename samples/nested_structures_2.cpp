#include <structopt/app.hpp>

struct CommandOptions {
  struct Sed : structopt::sub_command {
    // --trace
    std::optional<bool> trace = false;

    // remaining args
    std::vector<std::string> args;

    // pattern
    std::string pattern;

    // file
    std::string file;
  };
  Sed sed;
};
STRUCTOPT(CommandOptions::Sed, trace, args, pattern, file);
STRUCTOPT(CommandOptions, sed);

int main(int argc, char *argv[]) {

  auto app = structopt::app("my_app");

  try {

    auto options = app.parse<CommandOptions>(argc, argv);

    if (options.sed.has_value()) {
      // sed has been invoked

      if (options.sed.trace == true) {
        std::cout << "Trace enabled!\n";
      }

      std::cout << "Args    : ";
      for (auto &a : options.sed.args)
        std::cout << a << " ";
      std::cout << "\n";
      std::cout << "Pattern : " << options.sed.pattern << "\n";
      std::cout << "File    : " << options.sed.file << "\n";
    } else {
      std::cout << app.help();
    }

  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}