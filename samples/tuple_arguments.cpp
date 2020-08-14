#include <structopt/app.hpp>

struct CalculatorOptions {

  // types of operations supported
  enum class operation { add, subtract, multiply, divide };

  // single tuple positional argument
  std::tuple<operation, int, int> input;

};
STRUCTOPT(CalculatorOptions, input);



int main(int argc, char *argv[]) {

  auto app = structopt::app("my_app");

  try {
    auto options = app.parse<CalculatorOptions>(argc, argv);

    auto op = std::get<0>(options.input);
    auto lhs = std::get<1>(options.input);
    auto rhs = std::get<2>(options.input);
    switch(op)
    {
        case CalculatorOptions::operation::add:
            std::cout << lhs + rhs << "\n";
            break;
        case CalculatorOptions::operation::subtract:
            std::cout << lhs - rhs << "\n";
            break;
        case CalculatorOptions::operation::multiply:
            std::cout << lhs * rhs << "\n";
            break;
        case CalculatorOptions::operation::divide:
            std::cout << lhs / rhs << "\n";
            break;
    }
  }
  catch (std::exception& e) {
    std::cout << e.what();
    app.print_help();
  }

}