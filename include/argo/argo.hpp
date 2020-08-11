
#pragma once
#include <argo/json/json.hpp>
#include <argo/visit_struct/visit_struct.hpp>
#include <argo/is_stl_container.hpp>
#include <argo/argument_parser.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#define ARGO_SUBCOMMAND VISITABLE_STRUCT
#define ARGO_COMMAND VISITABLE_STRUCT

namespace argo {

template <typename T> T parse(int argc, char *argv[]) {
  T argument_struct;
  std::vector<std::string> arguments;
  for (std::size_t i = 0; i < argc; i++) {
    arguments.push_back(std::string(argv[i]));
  }

  argo::details::argument_parser argument_parser;
  argument_parser.arguments = std::move(arguments);
  visit_struct::for_each(argument_struct, argument_parser);

  return argument_struct;
}

} // namespace argo
