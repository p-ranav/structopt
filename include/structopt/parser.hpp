
#pragma once
#include <algorithm>
#include <array>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <structopt/array_size.hpp>
#include <structopt/is_number.hpp>
#include <structopt/is_specialization.hpp>
#include <structopt/sub_command.hpp>
#include <structopt/third_party/magic_enum/magic_enum.hpp>
#include <structopt/third_party/visit_struct/visit_struct.hpp>
#include <structopt/visitor.hpp>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>
#include <iterator>

namespace structopt {

namespace details {

struct parser {
  structopt::details::visitor visitor;
  std::vector<std::string> arguments;
  std::size_t current_index{1};
  std::size_t next_index{1};
  bool double_dash_encountered{false}; // "--" option-argument delimiter
  bool sub_command_invoked{false};
  std::string already_invoked_subcommand_name{""};

  bool is_optional(const std::string &name) {
    if (double_dash_encountered) {
      return false;
    } else if (name == "--") {
      double_dash_encountered = true;
      return false;
    } else if (is_valid_number(name)) {
      return false;
    }

    bool result = false;
    if (name.size() >= 2) {
      // e.g., -b, -v
      if (name[0] == '-') {
        result = true;

        // TODO: check if rest of name is NOT a decimal literal - this could be a negative
        // number if (name is a decimal literal) {
        //   result = false;
        // }

        if (name[1] == '-') {
          result = true;
        }
      }
    }
    return result;
  }

  bool is_optional_field(const std::string &next) {
    if (!is_optional(next)) {
      return false;
    }

    bool result = false;
    for (auto &field_name : visitor.field_names) {
      if (next == "--" + field_name or next == "-" + std::string(1, field_name[0])) {
        // okay `next` matches _a_ field name (which is an optional field)
        result = true;
      }
    }
    return result;
  }

  template <typename T>
  std::pair<T, bool> parse_argument(const char *name) {
    if (next_index >= arguments.size()) {
      return {T(), false};
    }
    T result;
    bool success = true;
    if constexpr (visit_struct::traits::is_visitable<T>::value) {
      result = parse_nested_struct<T>(name);
    }
    else if constexpr (std::is_enum<T>::value) {
      result = parse_enum_argument<T>(name);
      next_index += 1;
    } else if constexpr (structopt::is_specialization<T, std::pair>::value) {
      result =
          parse_pair_argument<typename T::first_type, typename T::second_type>(name);
    } else if constexpr (structopt::is_specialization<T, std::tuple>::value) {
      result = parse_tuple_argument<T>(name);
    } else if constexpr (!is_stl_container<T>::value) {
      result = parse_single_argument<T>(name);
      next_index += 1;
    } else if constexpr (structopt::is_array<T>::value) {
      constexpr std::size_t N = structopt::array_size<T>::size;
      result = parse_array_argument<typename T::value_type, N>(name);
    } else if constexpr (structopt::is_specialization<T, std::deque>::value 
      or structopt::is_specialization<T, std::list>::value
      or structopt::is_specialization<T, std::vector>::value) {
      result = parse_vector_like_argument<T>(name);
    } else if constexpr (
      structopt::is_specialization<T, std::set>::value 
      or structopt::is_specialization<T, std::multiset>::value
      or structopt::is_specialization<T, std::unordered_set>::value
      or structopt::is_specialization<T, std::unordered_multiset>::value) {
      result = parse_set_argument<T>(name);
    } else if constexpr (
      structopt::is_specialization<T, std::map>::value 
      or structopt::is_specialization<T, std::multimap>::value
      or structopt::is_specialization<T, std::unordered_map>::value
      or structopt::is_specialization<T, std::unordered_multimap>::value) {
      result = parse_map_argument<T>(name);
    } else if constexpr (
      structopt::is_specialization<T, std::queue>::value 
      or structopt::is_specialization<T, std::stack>::value
      or structopt::is_specialization<T, std::priority_queue>::value) {
      result = parse_container_adapter_argument<T>(name);
    } else {
      success = false;
    }
    return {result, success};
  }

  template <typename T> std::optional<T> parse_optional_argument(const char *name) {
    next_index += 1;
    std::optional<T> result;
    bool success;
    if (next_index < arguments.size()) {
      auto [value, success] = parse_argument<T>(name);
      if (success) {
        result = value;
      }
    }
    return result;
  }

  // Any field that can be constructed using std::stringstream
  // Not container type
  // Not a visitable type, i.e., a nested struct
  template <typename T>
  inline typename std::enable_if<!visit_struct::traits::is_visitable<T>::value, T>::type
  parse_single_argument(const char *name) {
    std::string argument = arguments[next_index];
    std::istringstream ss(argument);
    T result;

    if constexpr (std::is_integral<T>::value) {
      if (is_hex_notation(argument)) {
        ss >> std::hex >> result;
      }
      else if (is_octal_notation(argument)) {
        ss >> std::oct >> result;
      }
      else if (is_binary_notation(argument)) {
        argument.erase(0, 2); // remove "0b"
        result = std::stoi(argument, nullptr, 2);
      }
      else {
        ss >> std::dec >> result;
      }
    } else {
      ss >> result;
    }
    return result;
  }

  // Nested visitable struct
  template <typename T>
  inline typename std::enable_if<visit_struct::traits::is_visitable<T>::value, T>::type
  parse_nested_struct(const char *name) {

    if (!sub_command_invoked) {
      sub_command_invoked = true;
      already_invoked_subcommand_name = name;
    } else {
      // a sub-command has already been invoked
      throw std::runtime_error("Error: failed to invoke sub-command `"
        + std::string{name} + "` because a different sub-command, `"
        + already_invoked_subcommand_name 
        + "`, has already been invoked.");
    }

    T argument_struct;
    argument_struct.structopt_sub_command__name__ = name;

    if constexpr (std::is_base_of<structopt::sub_command, T>::value) {
      argument_struct.structopt_sub_command__invoked__ = true;
    }

    // Save struct field names
    visit_struct::for_each(argument_struct, argument_struct.structopt_sub_command__visitor__);

    structopt::details::parser parser;
    parser.next_index = 0;
    parser.current_index = 0;
    parser.double_dash_encountered = double_dash_encountered; 
    parser.visitor = argument_struct.structopt_sub_command__visitor__;

    std::copy(arguments.begin() + next_index, arguments.end(),
              std::back_inserter(parser.arguments));

    // std::cout << "Nested structures:\n";
    // for (auto& ns : parser.arguments) {
    //   std::cout << ns << " ";
    // }

    for (std::size_t i = 0; i < parser.arguments.size(); i++) {
      parser.current_index = i;
      visit_struct::for_each(argument_struct, parser);
    }

    if (!parser.visitor.positional_field_names.empty()) {
      // if all positional arguments were provided
      // this list would be empty
      auto front = parser.visitor.positional_field_names.front();
      if (std::find(parser.visitor.vector_like_positional_field_names.begin(),
                    parser.visitor.vector_like_positional_field_names.end(),
                    front) == 
          parser.visitor.vector_like_positional_field_names.end()) {
        // this positional argument is not a vector-like argument
        // it expects values
        throw std::runtime_error("Error: expected value for positional argument `" + front + "`.");
      }
    }

    // update current and next
    current_index += parser.next_index;
    next_index += parser.next_index;

    return argument_struct;
  }

  // Pair argument
  template <typename T1, typename T2>
  std::pair<T1, T2> parse_pair_argument(const char *name) {
    std::pair<T1, T2> result;
    bool success;
    {
      // Pair first
      auto [value, success] = parse_argument<T1>(name);
      if (success) {
        result.first = value;
      }
    }
    {
      // Pair second
      auto [value, success] = parse_argument<T2>(name);
      if (success) {
        result.second = value;
      }
    }
    return result;
  }

  // Array argument
  template <typename T, std::size_t N>
  std::array<T, N> parse_array_argument(const char *name) {
    std::array<T, N> result;
    bool success;

    const auto arguments_left = arguments.size() - next_index;
    if (arguments_left == 0 or arguments_left < N) {
      throw std::runtime_error("Error: expected " 
        + std::to_string(N) + " values for std::array argument `" + name 
        + "` - instead got only " + std::to_string(arguments_left) + " arguments.");
    }

    for (std::size_t i = 0; i < N; i++) {
      auto [value, success] = parse_argument<T>(name);
      if (success) {
        result[i] = value;
      }
    }
    return result;
  }

  // Map, Unordered_Map Argument
  template <typename T> T parse_map_argument(const char *name) {
    T result;
    // Parse from current till end
    for (std::size_t i = next_index; i < arguments.size(); i++) {
      const auto next = arguments[next_index];
      if (is_optional_field(next)) {
        // this marks the end of the container (break here)
        break;
      }
      result.insert(parse_pair_argument<typename T::key_type, typename T::mapped_type>(name));
    }
    return result;
  }

  template <class Tuple, class F, std::size_t... I>
  constexpr F for_each_impl(Tuple&& t, F&& f, std::index_sequence<I...>) {
      return (void)std::initializer_list<int>{(std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))),0)...}, f;
  }

  template <class Tuple, class F>
  constexpr F for_each(Tuple&& t, F&& f) {
      return for_each_impl(std::forward<Tuple>(t), std::forward<F>(f),
                          std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
  }

  // Parse single tuple element
  template <typename T>
  void parse_tuple_element(const char *name, T&& result) {
    auto [value, success] = parse_argument<typename std::remove_reference<T>::type>(name);
    if (success) {
      result = value;
    }
  }

  // Tuple argument
  template<typename Tuple>
  Tuple parse_tuple_argument(const char *name) {
    Tuple result;
    for_each(result, [&](auto&& arg) { 
      parse_tuple_element(name, arg);
    });
    return result;
  }

  // Vector, deque, list
  template <typename T> T parse_vector_like_argument(const char *name) {
    T result;
    // Parse from current till end
    for (std::size_t i = next_index; i < arguments.size(); i++) {
      const auto next = arguments[next_index];
      if (is_optional_field(next)) {
        // this marks the end of the container (break here)
        break;
      }
      auto [value, success] = parse_argument<typename T::value_type>(name);
      if (success) {
        result.push_back(value);
      }
    }
    return result;
  }

  // stack, queue, priority_queue
  template <typename T> T parse_container_adapter_argument(const char *name) {
    T result;
    // Parse from current till end
    for (std::size_t i = next_index; i < arguments.size(); i++) {
      const auto next = arguments[next_index];
      if (is_optional_field(next)) {
        // this marks the end of the container (break here)
        break;
      }
      auto [value, success] = parse_argument<typename T::value_type>(name);
      if (success) {
        result.push(value);
      }
    }
    return result;
  }

  // Set, multiset, unordered_set, unordered_multiset
  template <typename T> T parse_set_argument(const char *name) {
    T result;
    // Parse from current till end
    for (std::size_t i = next_index; i < arguments.size(); i++) {
      const auto next = arguments[next_index];
      if (is_optional_field(next)) {
        // this marks the end of the container (break here)
        break;
      }
      auto [value, success] = parse_argument<typename T::value_type>(name);
      if (success) {
        result.insert(value);
      }
    }
    return result;
  }

  // Enum class
  template <typename T> T parse_enum_argument(const char *name) {
    T result;
    auto maybe_enum_value = magic_enum::enum_cast<T>(arguments[next_index]);
    if (maybe_enum_value.has_value()) {
      result = maybe_enum_value.value();
    } else {
      constexpr auto allowed_names = magic_enum::enum_names<T>();

      std::string allowed_names_string = "";
      if (allowed_names.size()) {
        for (size_t i = 0; i < allowed_names.size() - 1; i++) {
          allowed_names_string += std::string{allowed_names[i]} + ", ";
        }
        allowed_names_string += allowed_names[allowed_names.size() - 1];
      }

      throw std::runtime_error("Error: unexpected input `"
                              + std::string{arguments[next_index]}
                              + "` provided for enum argument `" + std::string{name}
                              + "`. Allowed values are {" 
                              + allowed_names_string
                              + "}");
      // TODO: Throw error invalid enum option
    }
    return result;
  }

  // Visitor function for nested struct
  template <typename T>
  inline typename std::enable_if<visit_struct::traits::is_visitable<T>::value, void>::type
  operator()(const char *name, T &value) {
    // std::cout << "Parssing nested struct" << std::endl;
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];
      const auto field_name = std::string{name};

      // std::cout << "Next: " << next << "; Name: " << name << "\n";

      // Check if `next` is the start of a subcommand
      if (visitor.is_field_name(next) && next == field_name) {
        next_index += 1;
        value = parse_nested_struct<T>(name);
      }
    }
  }

  // Visitor function for any positional field (not std::optional)
  template <typename T>
  inline typename std::enable_if<!structopt::is_specialization<T, std::optional>::value &&
                                     !visit_struct::traits::is_visitable<T>::value,
                                 void>::type
  operator()(const char *name, T &result) {
    // std::cout << "Parsing positional: " << name << std::endl;
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];

      if (is_optional(next)) {
        return;
      }

      if (visitor.positional_field_names.empty()) {
        // We're not looking to save any more positional fields
        // all of them already have a value
        // TODO: Report error, unexpected argument
        return;
      }

      const auto field_name = visitor.positional_field_names.front();

      // // This will be parsed as a subcommand (nested struct)
      // if (visitor.is_field_name(next) && next == field_name) {
      //   return;
      // }

      if (field_name != std::string{name}) {
        // current field is not the one we want to parse 
        return;
      }

      // Remove from the positional field list as it is about to be parsed
      visitor.positional_field_names.pop_front();

      auto [value, success] = parse_argument<T>(field_name.c_str());
      if (success) {
        result = value;
      } else {
        // positional field does not yet have a value
        visitor.positional_field_names.push_front(field_name);
      }
    }
  }

  // Visitor function for std::optional field
  template <typename T>
  inline typename std::enable_if<structopt::is_specialization<T, std::optional>::value,
                                 void>::type
  operator()(const char *name, T &value) {
    // std::cout << "Parsing optional " << name << std::endl;
    if (next_index > current_index) {
      current_index = next_index;
    }

    if (current_index < arguments.size()) {
      const auto next = arguments[current_index];
      const auto field_name = std::string{name};

      if (next == "--" and double_dash_encountered == false) {
        double_dash_encountered = true;
        next_index += 1;
        return;
      }

      // Remove special characters from argument
      // e.g., --verbose => verbose
      // e.g., -v => v
      // e.g., --input-file => inputfile
      auto next_alpha = next;
      next_alpha.erase(std::remove_if(next_alpha.begin(), next_alpha.end(),
                                      [](char c) { return !std::isalpha(c); }),
                       next_alpha.end());

      // Remove special characters from field name
      // e.g., verbose => verbose
      // e.g., input_file => inputfile
      auto field_name_alpha = field_name;
      field_name_alpha.erase(std::remove_if(field_name_alpha.begin(),
                                            field_name_alpha.end(),
                                            [](char c) { return !std::isalpha(c); }),
                             field_name_alpha.end());

      // std::cout << "Trying to parse optional: " << field_name << " " << next << "\n";

      // if `next` looks like an optional argument
      // i.e., starts with `-` or `--`
      // see if you can find an optional field in the struct with a matching name

      // check if the current argument looks like it could be this optional field
      if ((double_dash_encountered == false) and
          ((next == "--" + field_name or next == "-" + std::string(1, field_name[0])) or
           (next_alpha == field_name_alpha))) {

        // std::cout << "Parsing optional: " << field_name << " " << next << "\n";

        // this is an optional argument matching the current struct field
        if constexpr (std::is_same<typename T::value_type, bool>::value) {
          // It is a boolean optional argument
          // Does it have a default value?
          // If yes, this is a FLAG argument, e.g,, "--verbose" will set it to true if the
          // default value is false No need to write "--verbose true"
          if (value.has_value()) {
            // The field already has a default value!
            value = !value.value(); // simply toggle it
            next_index += 1;
          } else {
            // boolean optional argument doesn't have a default value
            // expect one
            value = parse_optional_argument<typename T::value_type>(name);
          }
        } else {
          // Not std::optional<bool>
          // Parse the argument type <T>
          value = parse_optional_argument<typename T::value_type>(name);
        }
      }
      else {
        if (double_dash_encountered == false) {
          // A direct match of optional argument with field_name has not happened
          // This _could_ be a combined argument
          // e.g., -abc => -a, -b, and -c where each of these is a flag argument
          std::vector<std::string> potential_combined_argument;

          // if next is of the form `-abc` or `-de` and NOT of the form `--abc` 
          // `--abc` is not a combined argument
          // `-abc` might be
          if (next[0] == '-' and (next.size() > 1 and next[1] != '-')) {
            for (std::size_t i = 1; i < next.size(); i++) {
              potential_combined_argument.push_back("-" + std::string(1, next[i]));
            }
          }

          if (!potential_combined_argument.empty()) {
            bool is_combined_argument = true;
            for (auto& arg : potential_combined_argument) {
              if (!is_optional_field(arg)) {
                is_combined_argument = false;
                // TODO: report error unrecognized option in combined argument
              }
            }

            if (is_combined_argument) {

              // check and make sure the current field_name is 
              // in `potential_combined_argument`
              //
              // Let's say the argument `next` is `-abc`
              // the current field name is `b`
              // 1. Split `-abc` into `-a`, `-b`, and `-c`
              // 2. Check if `-b` is in the above list
              //    1. If yes, consider this as a combined argument
              //       push the list of arguments (split up) into `arguments`
              //    2. If no, nothing to do here
              bool field_name_matched = false;
              for (auto& arg : potential_combined_argument) {
                if (arg == "-" + std::string(1, field_name[0])) {
                  field_name_matched = true;
                }
              }

              if (field_name_matched) {
                // confirmed: this is a combined argument

                // insert the individual options that make up the combined argument
                // right after the combined argument
                // e.g., ""./main -abc" becomes "./main -abc -a -b -c"
                // Once this is done, increment `next_index` so that the parser loop will service
                // `-a`, `-b` and `-c` like any other optional arguments (flags and otherwise)
                for (std::vector<std::string>::reverse_iterator it = potential_combined_argument.rbegin(); 
                      it != potential_combined_argument.rend(); ++it) { 
                  auto& arg = *it;
                  if (next_index < arguments.size()) {
                    auto begin = arguments.begin();
                    arguments.insert(begin + next_index + 1, arg);
                  } else {
                    arguments.push_back(arg);
                  }
                }

                // get past the current combined argument
                next_index += 1;
              }
            }
          }
        }
      }
    }
  }
};

// Specialization for std::string
template <>
inline std::string parser::parse_single_argument<std::string>(const char *name) {
  return arguments[next_index];
}

// Specialization for bool
// yes, YES, on, 1, true, TRUE, etc. = true
// no, NO, off, 0, false, FALSE, etc. = false
// Converts argument to lower case before check
template <> inline bool parser::parse_single_argument<bool>(const char *name) {
  if (next_index > current_index) {
    current_index = next_index;
  }

  if (current_index < arguments.size()) {
    const std::vector<std::string> true_strings{"on", "yes", "1", "true"};
    const std::vector<std::string> false_strings{"off", "no", "0", "false"};
    std::string current_argument = arguments[current_index];

    // Convert argument to lower case
    std::transform(current_argument.begin(), current_argument.end(),
                   current_argument.begin(), ::tolower);

    // Detect if argument is true or false
    if (std::find(true_strings.begin(), true_strings.end(), current_argument) !=
        true_strings.end()) {
      return true;
    } else if (std::find(false_strings.begin(), false_strings.end(), current_argument) !=
               false_strings.end()) {
      return false;
    } else {
      // TODO: report error? Invalid argument, bool expected
      return false;
    }
  } else {
    return false;
  }
}

} // namespace details

} // namespace structopt
