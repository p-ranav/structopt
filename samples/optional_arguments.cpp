#include <structopt/app.hpp>

struct Options {
  std::optional<std::array<float, 3>> fixed_point;
};
STRUCTOPT(Options, fixed_point);

int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  if (options.fixed_point.has_value()) {
    std::cout << "Point: {" << options.fixed_point.value()[0] << ", " 
                            << options.fixed_point.value()[1] << ", "
                            << options.fixed_point.value()[2] << "}\n";
  }
}
