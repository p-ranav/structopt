
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

inline std::string string_to_kebab(std::string_view str) {
  // Generate kebab case and present as option
  std::string str2(str.begin(), str.end());
  details::string_replace(str2, "_", "-");
  return str2;
}

} // namespace details

} // namespace structopt