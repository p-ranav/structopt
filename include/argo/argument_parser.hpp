#pragma once
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace argo {

namespace details {

struct argument_parser {
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

}

}