
#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <structopt/is_stl_container.hpp>
#include <structopt/parser.hpp>
#include <structopt/third_party/visit_struct/visit_struct.hpp>
#include <type_traits>
#include <vector>

#define STRUCTOPT VISITABLE_STRUCT

namespace structopt {

class app {
  details::visitor visitor;

public:
  explicit app(const std::string name, const std::string version = "")
      : visitor(name, version) {}

  template <typename T>
  T parse(const std::vector<std::string> &arguments) {
    T argument_struct;

    // Visit the struct and save flag, optional and positional field names
    visit_struct::for_each(argument_struct, visitor);

    // Construct the argument parser
    structopt::details::parser parser;
    parser.visitor = visitor;
    parser.arguments = arguments;

    for (std::size_t i = 1; i < parser.arguments.size(); i++) {
      parser.current_index = i;
      visit_struct::for_each(argument_struct, parser);
    }

    if (!parser.visitor.positional_field_names.empty()) {
      // if all positional arguments were provided
      // this list would be empty
      auto front = parser.visitor.positional_field_names.front();
      if (std::find(parser.visitor.vector_like_positional_field_names.begin(),
                    parser.visitor.vector_like_positional_field_names.end(),
                    front) == 
          parser.visitor.vector_like_positional_field_names.end()) {
        // this positional argument is not a vector-like argument
        // it expects values
        throw structopt::exception("Error: expected value for positional argument `" + front + "`.", parser.visitor);
      }
    }

    return argument_struct;
  }

  template <typename T>
  T parse(int argc, char *argv[]) {
    std::vector<std::string> arguments;
    std::copy(argv, argv + argc, std::back_inserter(arguments));
    return parse<T>(arguments);
  }

  void print_help(std::ostream& os = std::cout) const {
    visitor.print_help(os);
  }
};

} // namespace structopt