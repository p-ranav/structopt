#pragma once
#include <argo/array_size.hpp>
#include <argo/is_specialization.hpp>
#include <array>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace argo {

namespace details {

struct argument_properties {

  enum class type {
    positional_single,
    positional_array,
    optional_single,
    optional_array
  };

  std::unordered_map<std::string, type> properties;

  template <typename T> void operator()(const char *name, T &value) {
    if constexpr (argo::is_specialization<T, std::optional>::value) {
      // optional argument
      if constexpr (!is_stl_container<typename T::value_type>::value) {
        // single 
        std::cout << name << " is optional" << "\n";
        properties.insert(std::make_pair(std::string{name}, type::optional_single));
      }
      else if constexpr (argo::is_array<typename T::value_type>::value) {
        // array
        properties.insert(std::make_pair(std::string{name}, type::optional_array));
      }
    }
    else {
      // positional
      if constexpr (!is_stl_container<T>::value) {
        // single
        std::cout << name << " is positional_single" << "\n";
        properties.insert(std::make_pair(std::string{name}, type::positional_single));
      }
      else if constexpr (argo::is_array<T>::value) {
        // array
        std::cout << name << " is positional_array" << "\n";
        properties.insert(std::make_pair(std::string{name}, type::positional_array));
      }
      else {
        // TODO: other types
      }
    }
  }
};

struct parser {
  std::vector<std::string> arguments;
  argument_properties properties;
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

  template <typename T>
  auto parse_optional_argument(const char * name) -> std::optional<T> {
    index += 1;
    std::optional<T> result;
    if (index < arguments.size()) {
      if constexpr (!is_stl_container<T>::value) { 
        result = parse_single_argument<T>(name);
      }
      else if constexpr (argo::is_array<T>::value) { 
        constexpr std::size_t N = argo::array_size<T>::size;
        result = parse_array_argument<typename T::value_type, N>(name);
      }
    }
    return result;
  }

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
      const auto next = arguments[index];

      if constexpr (argo::is_specialization<T, std::optional>::value) {
        value = parse_optional_argument<typename T::value_type>(name);
        index += 1;
      }
      else if constexpr (!is_stl_container<T>::value) { 
        value = parse_single_argument<T>(name);
        index += 1;
      }
      else if constexpr (argo::is_array<T>::value) { 
        constexpr std::size_t N = argo::array_size<T>::size;
        value = parse_array_argument<typename T::value_type, N>(name);
      }
      else {
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