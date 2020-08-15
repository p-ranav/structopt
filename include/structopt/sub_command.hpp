
#pragma once
#include <optional>
#include <structopt/visitor.hpp>

namespace structopt {

namespace details {
  struct parser;
}

class sub_command {
  std::optional<bool> invoked_;
  details::visitor visitor_;

  friend struct structopt::details::parser;

public:
  bool has_value() const { return invoked_.has_value(); }
};

} // namespace structopt