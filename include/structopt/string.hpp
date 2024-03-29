
#pragma once
#include <string>

namespace structopt {

namespace details {

static inline bool string_replace(std::string &str, const std::string &from,
                                  const std::string &to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

inline std::string string_to_kebab(std::string str) {
  // Generate kebab case and present as option
  details::string_replace(str, "_", "-");
  return str;
}

} // namespace details

} // namespace structopt