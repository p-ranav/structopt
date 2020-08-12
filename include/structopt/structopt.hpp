
#pragma once
#include <cstdlib>
#include <iostream>
#include <string>
#include <structopt/is_stl_container.hpp>
#include <structopt/parser.hpp>
#include <structopt/third_party/visit_struct/visit_struct.hpp>
#include <type_traits>
#include <vector>

#define STRUCTOPT VISITABLE_STRUCT

namespace structopt {

template <typename T> T parse(const std::vector<std::string> &arguments) {
  T argument_struct;

  structopt::details::parser parser;
  parser.arguments = std::move(arguments);

  for (std::size_t i = 1; i < parser.arguments.size(); i++) {
    parser.current_index = i;
    visit_struct::for_each(argument_struct, parser);
  }

  return argument_struct;
}

template <typename T> T parse(int argc, char *argv[]) {
  std::vector<std::string> arguments;
  std::copy(argv, argv + argc, std::back_inserter(arguments));
}

} // namespace structopt