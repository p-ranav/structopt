
#pragma once
#include <algorithm>
#include <string>
#include <queue>
#include <vector>

namespace structopt {

namespace details {

struct visitor {
  std::vector<std::string> field_names;
  std::deque<std::string> positional_field_names;
  std::deque<std::string> optional_field_names;
  std::deque<std::string> nested_struct_field_names;

  template <typename T>
  inline typename std::enable_if<structopt::is_specialization<T, std::optional>::value,
                                 void>::type
  operator()(const char *name, T &value) {
    field_names.push_back(name);
    optional_field_names.push_back(name);
  }

  // Visitor function for any positional field (not std::optional)
  template <typename T>
  inline typename std::enable_if<!structopt::is_specialization<T, std::optional>::value &&
                                     !visit_struct::traits::is_visitable<T>::value,
                                 void>::type
  operator()(const char *name, T &value) {
    field_names.push_back(name);
    positional_field_names.push_back(name);
  }

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