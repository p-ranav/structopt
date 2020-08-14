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

  auto app = structopt::app("my_app");

  try {
    auto options = app.parse<GrepOptions>(argc, argv);

    if (options.v == true) {
      std::cout << "`-v` provided - Matching is now reversed\n";
    }

    std::cout << "Search   : " << options.search << "\n";
    std::cout << "Pathspec : " << options.pathspec << "\n";
  }
  catch (std::exception& e) {
    std::cout << e.what();
    app.print_help();
  }

}