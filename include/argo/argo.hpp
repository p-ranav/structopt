
#pragma once
#include <argo/json/json.hpp>
#include <argo/visit_struct/visit_struct.hpp>
#include <argo/is_stl_container.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#define ARGO_SUBCOMMAND VISITABLE_STRUCT
#define ARGO_COMMAND VISITABLE_STRUCT

namespace argo {

namespace details {

template <typename T> void visit(T &config);

struct get_argument {

  // Subcommand - Nested struct
  template <typename T>
  inline typename std::enable_if<std::is_class<T>::value, void>::type
  _get_argument(const char *name, T &value) {
    std::cout << "Class type " << name << "\n";
    visit(value);
  }

  template <typename T>
  inline typename std::enable_if<!std::is_class<T>::value, void>::type
  _get_argument(const char *name, T &value) {
    std::cout << name << " " << value << "\n";
  }

  template <typename T> void operator()(const char *name, T &value) {
    // std::string field_name = name;
    _get_argument(name, value);
  }
};

template <typename T> void visit(T &argument_struct) {
  get_argument argument_visitor;
  visit_struct::for_each(argument_struct, argument_visitor);
}

}

template <typename T> T parse() {
  T result;
  details::visit(result);
  return result;
}

} // namespace argo
