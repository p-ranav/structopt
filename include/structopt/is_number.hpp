
#pragma once
#include <string>

namespace structopt {

namespace details {

static inline bool is_valid_number(const std::string & input) {
  std::size_t i = 0, j = input.length() - 1;

  // Handling whitespaces
  while (i < input.length() && input[i] == ' ')
    i++;
  while (j >= 0 && input[j] == ' ')
    j--;

  if (i > j)
    return false;

  // if string is of length 1 and the only
  // character is not a digit
  if (i == j && !(input[i] >= '0' && input[i] <= '9'))
    return false;

  // If the 1st char is not '+', '-', '.' or digit
  if (input[i] != '.' && input[i] != '+' && input[i] != '-' &&
      !(input[i] >= '0' && input[i] <= '9'))
    return false;

  // To check if a '.' or 'e' is found in given
  // string. We use this flag to make sure that
  // either of them appear only once.
  bool dot_or_exp = false;

  for (; i <= j; i++) {
    // If any of the char does not belong to
    // {digit, +, -, ., e}
    if (input[i] != 'e' && input[i] != '.' && input[i] != '+' && input[i] != '-' &&
        !(input[i] >= '0' && input[i] <= '9'))
      return false;

    if (input[i] == '.') {
      // checks if the char 'e' has already
      // occurred before '.' If yes, return false;.
      if (dot_or_exp == true)
        return false;

      // If '.' is the last character.
      if (i + 1 > input.length())
        return false;

      // if '.' is not followed by a digit.
      if (!(input[i + 1] >= '0' && input[i + 1] <= '9'))
        return false;
    }

    else if (input[i] == 'e') {
      // set dot_or_exp = 1 when e is encountered.
      dot_or_exp = true;

      // if there is no digit before 'e'.
      if (!(input[i - 1] >= '0' && input[i - 1] <= '9'))
        return false;

      // If 'e' is the last Character
      if (i + 1 > input.length())
        return false;

      // if e is not followed either by
      // '+', '-' or a digit
      if (input[i + 1] != '+' && input[i + 1] != '-' &&
          (input[i + 1] >= '0' && input[i] <= '9'))
        return false;
    }
  }

  /* If the string skips all above cases, then
  it is numeric*/
  return true;
}

} // namespace details

} // namespace structopt