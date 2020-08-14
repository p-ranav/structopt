#include <structopt/app.hpp>

struct Git {
  // Subcommand: git config
  struct Config : structopt::sub_command {
    std::optional<bool> global = false;
    std::array<std::string, 2> name_value_pair{};
  };
  Config config;

  // Subcommand: git init
  struct Init : structopt::sub_command {
    std::string name;
  };
  Init init;
};
STRUCTOPT(Git::Config, global, name_value_pair);
STRUCTOPT(Git::Init, name);
STRUCTOPT(Git, config, init);



int main(int argc, char *argv[]) {
  auto app = structopt::app("my_app");
  
  try {
    auto options = app.parse<Git>(argc, argv);

    if (options.config.has_value()) {
      // config was invoked
      std::cout << "You invoked `git config`:\n";
      std::cout << "Global : " << std::boolalpha << options.config.global.value() << "\n";
      std::cout << "Input  : (" << options.config.name_value_pair[0] << ", " << options.config.name_value_pair[1] << ")\n";
    }
    else if (options.init.has_value()) {
      // init was invoked
      std::cout << "You invoked `git init`:\n";
      std::cout << "Repository name : " << options.init.name << "\n";
    }


  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    app.print_help();
  }
}