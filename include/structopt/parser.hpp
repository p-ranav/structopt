#pragma once
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <structopt/array_size.hpp>
#include <structopt/is_specialization.hpp>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace structopt {

namespace details {

struct parser {
  std::vector<std::string> arguments;
  std::size_t current_index{1};
  std::size_t next_index{1};

  //   // Subcommand - Nested struct
  //   template <typename T>
  //   inline typename std::enable_if<std::is_class<T>::value, void>::type
  //   _get_argument(const char *name, T &value) {
  //     std::cout << "Class type " << name << "\n";
  //   }

  //   template <typename T>
  //   inline typename std::enable_if<!std::is_class<T>::value, void>::type
  //   _get_argument(const char *name, T &value) {
  //     std::cout << name << " " << value << "\n";
  //   }

  template <typename T> std::optional<T> parse_optional_argument(const char *name) {
    next_index += 1;
    std::optional<T> result;
    if (next_index < arguments.size()) {
      if constexpr (!is_stl_container<T>::value) {
        result = parse_single_argument<T>(name);
        next_index += 1;
      } else if constexpr (structopt::is_array<T>::value) {
        constexpr std::size_t N = structopt::array_size<T>::size;
        result = parse_array_argument<typename T::value_type, N>(name);
      }
    }
    return result;
  }

  // Any field that can be constructed using std::stringstream
  // Not container type
  template <typename T> T parse_single_argument(const char *name) {
    const std::string argument = arguments[next_index];
    std::istringstream ss(argument);
    T result;
    ss >> result;
    return result;
  }

  // Array argument
  template <typename T, std::size_t N> std::array<T, N> parse_array_argument(const char *name) {
    std::array<T, N> result;
    for (std::size_t i = 0; i < N; i++) {
      // TODO: check index to see if N arguments are available to parse
      if constexpr (!is_stl_container<T>::value) {
        result[i] = parse_single_argument<T>(name);
        next_index += 1;
      } else if constexpr (structopt::is_array<T>::value) {
        constexpr std::size_t NESTED_N = structopt::array_size<T>::size;
        result[i] = parse_array_argument<typename T::value_type, NESTED_N>(name);
      }
    }
    return result;
  }

  // Visitor function for any positional field (not std::optional)
  template <typename T>
  inline typename std::enable_if<!structopt::is_specialization<T, std::optional>::value, void>::type
  operator()(const char *name, T &value) {
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];
      const auto field_name = std::string{name};

      // TODO: Deal with negative numbers - these are not optional arguments
      if ((next.size() >= 1 and next[0] == '-') or
          (next.size() >= 2 and next[0] == '-' and next[1] == '-')) {
        return;
      }

      if constexpr (!is_stl_container<T>::value) {
        value = parse_single_argument<T>(name);
        next_index += 1;
      } else if constexpr (structopt::is_array<T>::value) {
        constexpr std::size_t N = structopt::array_size<T>::size;
        value = parse_array_argument<typename T::value_type, N>(name);
      }
    }
  }

  // Visitor function for std::optional field
  template <typename T>
  inline typename std::enable_if<structopt::is_specialization<T, std::optional>::value, void>::type
  operator()(const char *name, T &value) {
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];
      const auto field_name = std::string{name};

      // if `next` looks like an optional argument
      // i.e., starts with `-` or `--`
      // see if you can find an optional field in the struct with a matching name

      // check if the current argument looks like it could be this optional field
      if (next == "--" + field_name or next == "-" + std::string(1, field_name[0])) {
        // this is an optional argument matching the current struct field
        value = parse_optional_argument<typename T::value_type>(name);
      }
    }
  }
};

// Specialization for std::string
template <> inline std::string parser::parse_single_argument<std::string>(const char *name) {
  return arguments[next_index];
}

// Specialization for bool
// yes, YES, on, 1, true, TRUE, etc. = true
// no, NO, off, 0, false, FALSE, etc. = false
// Converts argument to lower case before check
template <> inline bool parser::parse_single_argument<bool>(const char *name) {
  if (next_index > current_index) {
    current_index = next_index;
  }

  if (current_index < arguments.size()) {
    const std::vector<std::string> true_strings{"on", "yes", "1", "true"};
    const std::vector<std::string> false_strings{"off", "no", "0", "false"};
    std::string current_argument = arguments[current_index];

    // Convert argument to lower case
    std::transform(current_argument.begin(), current_argument.end(), current_argument.begin(),
                   ::tolower);

    // Detect if argument is true or false
    if (std::find(true_strings.begin(), true_strings.end(), current_argument) !=
        true_strings.end()) {
      return true;
    } else if (std::find(false_strings.begin(), false_strings.end(), current_argument) !=
               false_strings.end()) {
      return false;
    } else {
      // TODO: report error? Invalid argument, bool expected
      return false;
    }
  } else {
    return false;
  }
}

} // namespace details

} // namespace structopt