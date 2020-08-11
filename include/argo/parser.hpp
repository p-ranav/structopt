#pragma once
#include <argo/array_size.hpp>
#include <array>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace argo {

namespace details {

struct parser {
  std::vector<std::string> arguments;
  std::size_t index{1};

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

  // Any field that can be constructed using std::stringstream
  // Not container type
  template <typename T>
  T parse_single_argument(const char * name) {
    const std::string argument = arguments[index];
    std::istringstream ss(argument);
    T result;
    ss >> result;
    return result;
  }

  // Array argument
  template <typename T, std::size_t N>
  std::array<T, N> parse_array_argument(const char * name) {
    std::cout << "Parsing array\n";
    std::array<T, N> result;
    for (std::size_t i = 0; i < N; i++) {
      // TODO: check index to see if N arguments are available to parse
      result[i] = parse_single_argument<T>(name);
      index += 1;
    }
    return result;
  }

  template <typename T> void operator()(const char *name, T &value) {
    if (index < arguments.size()) {
      // more arguments available
      if constexpr (!is_stl_container<T>::value) {
        value = parse_single_argument<T>(name);
        index += 1;
      }
      else if constexpr (argo::is_array<T>::value) {
        constexpr std::size_t N = argo::array_size<T>::size;
        value = parse_array_argument<typename T::value_type, N>(name);
      } else {
        // std::cout << "Container but not std::array\n";
      }
    }
  }
};

// Specialization for bool
// yes, YES, on, 1, true, TRUE, etc. = true
// no, NO, off, 0, false, FALSE, etc. = false
// Converts argument to lower case before check
template <>
inline bool parser::parse_single_argument<bool>(const char * name) {
  const std::vector<std::string> true_strings{"on", "yes", "1", "true"};
  const std::vector<std::string> false_strings{"off", "no", "0", "false"};
  std::string current_argument = arguments[index];

  // Convert argument to lower case
  std::transform(current_argument.begin(), current_argument.end(), current_argument.begin(),
                  ::tolower);

  // Detect if argument is true or false
  if (std::find(true_strings.begin(), true_strings.end(), current_argument) != true_strings.end()) {
    return true;
  } 
  else if (std::find(false_strings.begin(), false_strings.end(), current_argument) != false_strings.end()) {
    return false;
  }
  else {
    // TODO: report error? Invalid argument, bool expected
    return false;
  }
}

}

}