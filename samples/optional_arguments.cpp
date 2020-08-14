#include <structopt/app.hpp>

struct Options {
  std::optional<std::array<float, 3>> fixed_point = std::array<float, 3>{0.0f, 0.0f, 0.0f};
};
STRUCTOPT(Options, fixed_point);

int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  std::cout << "Point: {" << options.fixed_point.value()[0] << ", " 
                          << options.fixed_point.value()[1] << ", "
                          << options.fixed_point.value()[2] << "}\n";
}
