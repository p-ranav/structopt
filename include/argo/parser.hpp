#pragma once
#include <algorithm>
#include <sstream>
#include <string>
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

  template <typename T> void operator()(const char *name, T &value) {
    if (index < arguments.size()) {
      // more arguments available
      if (!is_stl_container<T>::value) {
        value = parse_single_argument<T>(name);
      }
      index += 1;
    }
  }
};

// Specialization for bool
// yes, YES, on, 1, true = true
// no, NO, off, 0, false = false
template <>
bool parser::parse_single_argument<bool>(const char * name) {
  const std::vector<std::string> true_strings{"on", "ON", "yes", "YES", "1", "true", "TRUE"};
  const std::vector<std::string> false_strings{"off", "OFF", "no", "NO", "0", "false", "FALSE"};
  const auto current_argument = arguments[index];
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