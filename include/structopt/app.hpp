
#pragma once
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <structopt/app.hpp>
#include <structopt/is_stl_container.hpp>
#include <structopt/parser.hpp>
#include <structopt/string.hpp>
#include <structopt/third_party/visit_struct/visit_struct.hpp>
#include <type_traits>
#include <vector>

#define STRUCTOPT VISITABLE_STRUCT

namespace structopt {

class app {
  std::string name_;
  std::string version_;
  details::visitor visitor;

public:
  explicit app(const std::string name, const std::string version = "")
      : name_(name), version_(version) {}

  std::string get_name() const { return name_; }

  std::string get_version() const { return version_; }

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

    return argument_struct;
  }

  template <typename T>
  T parse(int argc, char *argv[]) {
    std::vector<std::string> arguments;
    std::copy(argv, argv + argc, std::back_inserter(arguments));
    return parse<T>(arguments);
  }

  void generate_help(std::ostream& os) {
    os << name_ << " [FLAGS] [OPTIONS] ";
    for (auto& field : visitor.positional_field_names) {
      os << field << " ";
    }

    os << "\n\nFLAGS:\n";
    for (auto& flag : visitor.flag_field_names) {
      os << "    -" << flag[0] << ", --" << flag << "\n";
    }

    os << "\nOPTIONS:\n";
    for (auto& option : visitor.optional_field_names) {

      // Generate kebab case and present as option
      auto kebab_case = option;
      details::string_replace(kebab_case, "_", "-");
      std::string long_form = "";
      if (kebab_case != option) {
        long_form = kebab_case;
      } else {
        long_form = option;
      }

      os << "    -" << option[0] << ", --" << long_form << " <" << option << ">" << "\n";
    }

    os << "\nARGS:\n";
    for (auto& arg : visitor.positional_field_names) {
      os << "    " << arg << "\n";
    }
    os << "\n";
  }
};

} // namespace structopt