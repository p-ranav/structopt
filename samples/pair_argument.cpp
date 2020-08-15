#include <structopt/app.hpp>

struct Options {
  std::pair<std::string, std::string> name;
};
STRUCTOPT(Options, name);

int main(int argc, char *argv[]) {

  try {
    auto options = structopt::app("my_app").parse<Options>(argc, argv);

    std::cout << "First name : " << options.name.first << "\n";
    std::cout << "Last name  : " << options.name.second << "\n";
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}