
#pragma once
#include <exception>
#include <sstream>
#include <string>
#include <structopt/visitor.hpp>

namespace structopt {

class exception : public std::exception {
  std::string what_{""};
  std::string help_{""};
  details::visitor visitor_;

public:

  exception(const std::string & what, const details::visitor& visitor) : what_(what), help_(""), visitor_(visitor) {
    std::stringstream os;
    visitor_.print_help(os);
    help_ = os.str();
  }

  const char * what() const throw () {
    return what_.c_str();
  }

  const char * help() const throw() {
    return help_.c_str();
  }
};

}