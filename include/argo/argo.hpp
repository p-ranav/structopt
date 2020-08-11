
#pragma once
#include <argo/third_party/visit_struct/visit_struct.hpp>
#include <argo/is_stl_container.hpp>
#include <argo/parser.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#define ARGO_STRUCT VISITABLE_STRUCT

namespace argo {

template <typename T> T parse(const std::vector<std::string> & arguments) {
  T argument_struct;
  
  argo::details::parser parser;
  parser.arguments = std::move(arguments);

  for (std::size_t i = 1; i < parser.arguments.size(); i++) {
    parser.current_index = i;
    visit_struct::for_each(argument_struct, parser);
  }

  return argument_struct;
}

template <typename T> T parse(int argc, char *argv[]) {
  std::vector<std::string> arguments;
  std::copy(argv, argv + argc, std::back_inserter(arguments));
}

} // namespace argo

// for each argument
// * is argument start of field value?
//   yes:
//     * which field is it?
//       * the next positional field
//         * need to keep track of next positional field
//       * a specific optional field - argument matches optional field name
//   no:
//     * it is the value of an optional field, e.g., --verbose true
//     * it is the value of an array field, e.g., 1 2 3 4 5
//
// for each argument
//   1. get argument value
//   2. visit each struct field
//   3. does value start with `-` or `--` and match an optional field name? ok cool parse optional field and save value
//   4. no? it is the value of the next positional argument

// ./main 1 3.14 true a b c --blah 1 2 3 --verbose true
// foo[int] bar[float] baz[bool] chars[3] blah[3] verbose[bool]
// 1 = foo
// 3.14 = bar
// true = baz
// a = 