
#pragma once
#include <algorithm>
#include <string>
#include <queue>
#include <vector>

namespace structopt {

class app;

namespace details {

struct visitor {
  std::vector<std::string> field_names;
  std::deque<std::string> positional_field_names;
  std::deque<std::string> vector_like_positional_field_names;
  std::deque<std::string> flag_field_names;
  std::deque<std::string> optional_field_names;
  std::deque<std::string> nested_struct_field_names;

  // Visitor function for std::optional - could be an option or a flag
  template <typename T>
  inline typename std::enable_if<structopt::is_specialization<T, std::optional>::value,
                                 void>::type
  operator()(const char *name, T &value) {
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
  operator()(const char *name, T &value) {
    field_names.push_back(name);
    positional_field_names.push_back(name);
    if constexpr (structopt::is_specialization<T, std::deque>::value 
      or structopt::is_specialization<T, std::list>::value
      or structopt::is_specialization<T, std::vector>::value) {
      // keep track of vector-like fields as these (even though positional) 
      // can be happy without any arguments
      vector_like_positional_field_names.push_back(name);
    }
  }

  // Visitor function for nested structs
  template <typename T>
  inline typename std::enable_if<visit_struct::traits::is_visitable<T>::value,
                                 void>::type
  operator()(const char *name, T &value) {
    field_names.push_back(name);
    nested_struct_field_names.push_back(name);
  }

  bool is_field_name(const std::string &name) {
    return std::find(field_names.begin(), field_names.end(), name) != field_names.end();
  }
};

} // namespace details

} // namespace structopt