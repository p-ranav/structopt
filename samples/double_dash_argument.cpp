#include <structopt/app.hpp>

struct GrepOptions {
  // reverse the matching
  // enable with `-v`
  std::optional<bool> v = false;
  
  // positional arguments
  std::string search;
  std::string pathspec;
};
STRUCTOPT(GrepOptions, v, search, pathspec);



int main(int argc, char *argv[]) {

  try {
    auto options = structopt::app("my_app").parse<GrepOptions>(argc, argv);

    if (options.v == true) {
      std::cout << "`-v` provided - Matching is now reversed\n";
    }

    std::cout << "Search   : " << options.search << "\n";
    std::cout << "Pathspec : " << options.pathspec << "\n";
  }
 catch (structopt::exception& e) {
    std::cout << e.what() << "\n";
    std::cout << e.help() << "\n";
  }

}