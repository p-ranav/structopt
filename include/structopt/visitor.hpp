#pragma once
#include <algorithm>
#include <string>
#include <vector>

namespace structopt {

namespace details {

struct visitor {
  std::vector<std::string> field_names;

  template <typename T> void operator()(const char *name, T &value) {
    field_names.push_back(name);
  }

  bool is_field_name(const std::string &name) {
    return std::find(field_names.begin(), field_names.end(), name) != field_names.end();
  }
};

} // namespace details

} // namespace structopt