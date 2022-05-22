
#pragma once
#include <algorithm>
#include <iostream>
#include <optional>
#include <queue>
#include <string>
#include <structopt/is_specialization.hpp>
#include <structopt/string.hpp>
#include <structopt/third_party/visit_struct/visit_struct.hpp>
#include <type_traits>
#include <vector>

namespace structopt {

class app;

namespace details {

struct visitor {
  std::string name;
  std::string version;
  std::optional<std::string> help;
  std::vector<std::string> field_names;
  std::deque<std::string> positional_field_names; // mutated by parser
  std::deque<std::string> positional_field_names_for_help;
  std::deque<std::string> vector_like_positional_field_names;
  std::deque<std::string> flag_field_names;
  std::deque<std::string> optional_field_names;
  std::deque<std::string> nested_struct_field_names;

  visitor() = default;

  explicit visitor(const std::string &name, const std::string &version)
      : name(name), version(version) {}

  explicit visitor(const std::string &name, const std::string &version, const std::string& help)
      : name(name), version(version), help(help) {}

  // Visitor function for std::optional - could be an option or a flag
  template <typename T>
  inline typename std::enable_if<structopt::is_specialization<T, std::optional>::value,
                                 void>::type
  operator()(const char *name, T &) {
    field_names.push_back(name);
    if constexpr (std::is_same<typename T::value_type, bool>::value) {
      flag_field_names.push_back(name);
    } else {
      optional_field_names.push_back(name);
    }
  }

  // Visitor function for any positional field (not std::optional)
  template <typename T>
  inline typename std::enable_if<!structopt::is_specialization<T, std::optional>::value &&
                                     !visit_struct::traits::is_visitable<T>::value,
                                 void>::type
  operator()(const char *name, T &) {
    field_names.push_back(name);
    positional_field_names.push_back(name);
    positional_field_names_for_help.push_back(name);
    if constexpr (structopt::is_specialization<T, std::deque>::value ||
                  structopt::is_specialization<T, std::list>::value ||
                  structopt::is_specialization<T, std::vector>::value ||
                  structopt::is_specialization<T, std::set>::value ||
                  structopt::is_specialization<T, std::multiset>::value ||
                  structopt::is_specialization<T, std::unordered_set>::value ||
                  structopt::is_specialization<T, std::unordered_multiset>::value ||
                  structopt::is_specialization<T, std::queue>::value ||
                  structopt::is_specialization<T, std::stack>::value ||
                  structopt::is_specialization<T, std::priority_queue>::value) {
      // keep track of vector-like fields as these (even though positional)
      // can be happy without any arguments
      vector_like_positional_field_names.push_back(name);
    }
  }

  // Visitor function for nested structs
  template <typename T>
  inline typename std::enable_if<visit_struct::traits::is_visitable<T>::value, void>::type
  operator()(const char *name, T &) {
    field_names.push_back(name);
    nested_struct_field_names.push_back(name);
  }

  bool is_field_name(const std::string &field_name) {
    return std::find(field_names.begin(), field_names.end(), field_name) !=
           field_names.end();
  }

  void print_help(std::ostream &os) const {
    if (help.has_value() && help.value().size() > 0) {
      os << help.value();
    } else {
      os << "\nUSAGE: " << name << " ";

      if (not flag_field_names.empty()) {
        os << "[FLAGS] ";
      }

      if (not optional_field_names.empty()) {
        os << "[OPTIONS] ";
      }

      if (not nested_struct_field_names.empty()) {
        os << "[SUBCOMMANDS] ";
      }

      for (auto &field : positional_field_names_for_help) {
        os << field << " ";
      }

      if (not flag_field_names.empty()) {
        os << "\n\nFLAGS:\n";
        for (auto &flag : flag_field_names) {
          os << "    -" << flag[0] << ", --" << flag << "\n";
        }
      } else {
        os << "\n";
      }

      if (not optional_field_names.empty()) {
        os << "\nOPTIONS:\n";
        for (auto &option : optional_field_names) {

          // Generate kebab case and present as option
          auto kebab_case = option;
          details::string_replace(kebab_case, "_", "-");
          std::string long_form = "";
          if (kebab_case != option) {
            long_form = kebab_case;
          } else {
            long_form = option;
          }

          os << "    -" << option[0] << ", --" << long_form << " <" << option << ">"
            << "\n";
        }
      }

      if (not nested_struct_field_names.empty()) {
        os << "\nSUBCOMMANDS:\n";
        for (auto &sc : nested_struct_field_names) {
          os << "    " << sc << "\n";
        }
      }

      if (not positional_field_names_for_help.empty()) {
        os << "\nARGS:\n";
        for (auto &arg : positional_field_names_for_help) {
          os << "    " << arg << "\n";
        }
      }
    }
  }
};

} // namespace details

} // namespace structopt
