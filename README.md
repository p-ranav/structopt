<p align="center">
  <img height="70" src="img/logo.png"/>
</p>

<p align="center">
  Parse command line arguments by defining a struct
</p>

<p align="center">
  <a href="https://github.com/p-ranav/structopt/releases">
    <img src="https://img.shields.io/github/release/p-ranav/structopt.svg" alt="ci status"/>
  </a>
  <a href="https://conan.io/center/recipes/structopt">
    <img src="https://img.shields.io/conan/v/structopt" alt="conan package"/>
  </a>
  <a href="https://travis-ci.com/p-ranav/structopt">
    <img src="https://travis-ci.com/p-ranav/structopt.svg?branch=master" alt="ci status"/>
  </a>
  <a href="https://ci.appveyor.com/project/p-ranav/structopt/branch/master">
    <img src="https://ci.appveyor.com/api/projects/status/fyufe5f11mhahwq9/branch/master?svg=true" alt="ci status"/>
  </a>
  <a href="https://www.codacy.com/manual/p-ranav/structopt?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=p-ranav/structopt&amp;utm_campaign=Badge_Grade">
    <img src="https://app.codacy.com/project/badge/Grade/e6e76a680e61445a90616d27cb69b927" alt="codacy"/>
  </a>
  <a href="https://en.wikipedia.org/wiki/C%2B%2B17">
    <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="standard"/>
  </a>
  <a href="https://github.com/p-ranav/tabulate/blob/master/LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="license"/>
  </a>
</p>

## Quick Start

```cpp
#include <structopt/app.hpp>

struct Options {
   // positional argument
   //   e.g., ./main <file>
   std::string config_file;

   // optional argument
   //   e.g., -b "192.168.5.3"
   //   e.g., --bind_address "192.168.5.3"
   //
   // options can be delimited with `=` or `:`
   // note: single dash (`-`) is enough for short & long option
   //   e.g., -bind_address=localhost
   //   e.g., -b:192.168.5.3
   //
   // the long option can also be provided in kebab case:
   //   e.g., --bind-address 192.168.5.3
   std::optional<std::string> bind_address;

   // flag argument
   // Use `std::optional<bool>` and provide a default value.
   //   e.g., -v
   //   e.g., --verbose
   //   e.g., -verbose
   std::optional<bool> verbose = false;

   // directly define and use enum classes to limit user choice
   //   e.g., --log-level debug
   //   e.g., -l error
   enum class LogLevel { debug, info, warn, error, critical };
   std::optional<LogLevel> log_level = LogLevel::info;

   // pair argument
   // e.g., -u <first> <second>
   // e.g., --user <first> <second>
   std::optional<std::pair<std::string, std::string>> user;

   // use containers like std::vector
   // to collect "remaining arguments" into a list
   std::vector<std::string> files;
};
STRUCTOPT(Options, config_file, bind_address, verbose, log_level, user, files);
```

Create a `structopt::app` and parse the command line arguments into the `Options` struct:

```cpp
int main(int argc, char *argv[]) {

  try {

    // Line of code that does all the work:
    auto options = structopt::app("my_app").parse<Options>(argc, argv);

    // Print out parsed arguments:

    // std::cout << "config_file  = " << options.config_file << "\n";
    // std::cout << "bind_address = " << options.bind_address.value_or("not provided") << "\n";
    // std::cout << "verbose      = " << std::boolalpha << options.verbose.value() << "\n";
    // ...

  } catch (structopt::exception& e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

Now let's pass some arguments to this program:

```console
foo@bar:~$ ./main config.csv file5.csv file6.json
config_file  = config.csv
bind_address = not provided
verbose      = false
log_level    = 1
user         = not provided
files        = { file5.csv file6.json }

foo@bar:~$ ./main config.csv --bind-address localhost:9000 -v -log-level error file1.txt file2.txt
config_file  = config.csv
bind_address = localhost:9000
verbose      = true
log_level    = 3
user         = not provided
files        = { file1.txt file2.txt }

foo@bar:~$ ./main config_2.csv --bind-address 192.168.7.3 -log-level debug file1.txt file3.txt file4.txt --user "John Doe" "john.doe@foo.com"
config_file  = config_2.csv
bind_address = 192.168.7.3
verbose      = false
log_level    = 0
user         = John Doe<john.doe@foo.com>
files        = { file1.txt file3.txt file4.txt }
```

## Table of Contents

*    [Getting Started](#getting-started)
     *    [Positional Arguments](#positional-arguments)
     *    [Optional Arguments](#optional-arguments)
          *    [Double dash (`--`) Argument](#double-dash----argument)
     *    [Flag Arguments](#flag-arguments)
     *    [Enum Class Arguments (Choices)](#enum-class-arguments)
     *    [Tuple Arguments](#tuple-arguments)
     *    [Vector Arguments](#vector-arguments)
     *    [Compound Arguments](#compound-arguments)
     *    [Parsing Numbers](#parsing-numbers)
          *    [Integer Literals](#integer-literals)
          *    [Floating point Literals](#floating-point-literals)
     *    [Nested Structures (Sub-commands)](#nested-structures)
     *    [Sub-Commands, Vector Arguments, and Delimited Positional Arguments](#sub-commands-vector-arguments-and-delimited-positional-arguments)
     *    [Printing Help](#printing-help)
     *    [Printing CUSTOM Help](#printing-custom-help)
*    [Building Samples and Tests](#building-samples-and-tests)
*    [Compiler Compatibility](#compiler-compatibility)
*    [Generating Single Header](#generating-single-header)
*    [Contributing](#contributing)
*    [License](#license)

## Getting Started

`structopt` is a header-only library. Just add `include/` to your _include_directories_ and you should be good to go. A single header file version is also available in `single_include/`.

### Positional Arguments

Here's an example of two positional arguments: `input_file` and `output_file`. `input_file` is expected to be the first argument and `output_file` is expected to be the second argument

```cpp
#include <structopt/app.hpp>

struct FileOptions {
  // Positional arguments
  // ./main <input_file> <output_file>
  std::string input_file;
  std::string output_file;
};
STRUCTOPT(FileOptions, input_file, output_file);



int main(int argc, char *argv[]) {

  try {
    auto options = structopt::app("my_app").parse<FileOptions>(argc, argv);

    // Print parsed arguments:
    std::cout << "\nInput file  : " << options.input_file << "\n";
    std::cout << "Output file : " << options.output_file << "\n";

  } catch (structopt::exception& e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

```console
foo@bar:~$ ./main foo.txt bar.csv

Input file  : foo.txt
Output file : bar.csv

foo@bar:~$ ./main foo.csv
Error: expected value for positional argument `output_file`.

USAGE: ./my_app input_file output_file

ARGS:
    input_file
    output_file
```

### Optional Arguments

Now, let's look at optional arguments. To configure an optional argument, use `std::optional` in the options struct like below.

```cpp
#include <structopt/app.hpp>

struct GccOptions {
  // language standard
  // e.g., -std=c++17
  // e.g., --std c++20
  std::optional<std::string> std = "c++11";

  // verbosity enabled with `-v` or `--verbose`
  // or `-verbose`
  std::optional<bool> verbose = false;

  // enable all warnings with `-Wall`
  std::optional<bool> Wall = false;

  // produce only the compiled code
  // e.g., gcc -C main.c
  std::optional<bool> Compile = false;

  // produce output with `-o <exec_name>`
  std::optional<std::string> output = "a.out";

  std::string input_file;
};
STRUCTOPT(GccOptions, std, verbose, Wall, Compile, output, input_file);


int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("gcc").parse<GccOptions>(argc, argv);

    // Print parsed arguments

    std::cout << "std        : " << options.std.value() << "\n";
    std::cout << "verbose    : " << std::boolalpha << options.verbose.value() << "\n";
    std::cout << "Wall       : " << std::boolalpha << options.Wall.value() << "\n";
    std::cout << "Compile    : " << std::boolalpha << options.Compile.value() << "\n";
    std::cout << "Output     : " << options.output.value() << "\n";
    std::cout << "Input file : " << options.input_file << "\n";
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

***NOTE*** `structopt` supports two option delimiters, `=` and `:` for optional arguments. This is meaningful and commonly used in single-valued optional arguments, e.g., `--std=c++17`.

```console
foo@bar:~$ ./main -C main.cpp
std        : c++11
verbose    : false
Wall       : false
Compile    : true
Output     : a.out
Input file : main.cpp

foo@bar:~$ ./main -std=c++17 -o main main.cpp
std        : c++17
verbose    : false
Wall       : false
Compile    : false
Output     : main
Input file : main.cpp

foo@bar:~$ ./main main.cpp -v -std:c++14 --output:main -Wall
std        : c++14
verbose    : true
Wall       : true
Compile    : false
Output     : main
Input file : main.cpp
```

***NOTE*** In summary, for a field in your struct named `bind_address`, the following are all legal ways to provide a value:

* Short form:
  * `-b <value>`
* Long form:
  * `--bind_address <value>`
  * `-bind_address <value>`
* Kebab case:
  * `--bind-address <value>`
  * `-bind-address <value>`
* Equal (`'='`) option delimiter
  * `-b=<value>`
  * `--bind_address=<value>`
  * `-bind_address=<value>`
  * `--bind-address=<value>`
  * `-bind-address=<value>`
* Colon `':'` option delimiter
  * `-b:<value>`
  * `--bind_address:<value>`
  * `-bind_address:<value>`
  * `--bind-address:<value>`
  * `-bind-address:<value>`

#### Double dash (`--`) Argument

A double dash (`--`) is used in most bash built-in commands and many other commands to signify the end of command options, after which only positional parameters are accepted.

Example use: lets say you want to `grep` a file for the string `-v` - normally `-v` will be considered the option to reverse the matching meaning (only show lines that do not match), but with `--` you can `grep` for string `-v` like this:

```cpp
#include <structopt/app.hpp>

struct GrepOptions {
  // reverse the matching
  // enable with `-v`
  std::optional<bool> v = false;

  // positional arguments
  std::string search;
  std::string pathspec;
};
STRUCTOPT(GrepOptions, v, search, pathspec);



int main(int argc, char *argv[]) {

  try {
    auto options = structopt::app("my_app").parse<GrepOptions>(argc, argv);

    if (options.v == true) {
      std::cout << "`-v` provided - Matching is now reversed\n";
    }

    std::cout << "Search   : " << options.search << "\n";
    std::cout << "Pathspec : " << options.pathspec << "\n";
  }
  catch (structopt::exception& e) {
    std::cout << e.what();
    std::cout << e.help();
  }

}
```

```console
foo@bar:~$ ./main -v foo bar.txt
`-v` provided - Matching is now reversed
Search   : foo
Pathspec : bar.txt

foo@bar:~$ ./main -- -v bar.txt
Search   : -v
Pathspec : bar.txt
```

### Flag Arguments

Flag arguments are `std::optional<bool>` with a default value.

***NOTE*** The default value here is important. It is not a flag if a default value isn't provided. It will simply be an optional argument.

***NOTE*** If `--verbose` is a flag argument with a default value of `false`, then providing the argument will set it to `true`. If `--verbose` does not have a default value, then `structopt` will expect the user to provide a value, e.g., `--verbose true`.

```cpp
#include <structopt/app.hpp>

struct Options {
  // verbosity flag
  // -v, --verbose
  // remember to provide a default value
  std::optional<bool> verbose = false;
};
STRUCTOPT(Options, verbose);



int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  if (options.verbose == true) {
    std::cout << "Verbosity enabled\n";
  }
}
```

```console
foo@bar:~$ ./main

foo@bar:~$ ./main -v
Verbosity enabled

foo@bar:~$ ./main --verbose
Verbosity enabled
```

### Enum Class Arguments

Thanks to [magic_enum](https://github.com/Neargye/magic_enum), `structopt` supports enum classes. You can use an enum classes to ask the user to provide a value given a choice of values, restricting the possible set of allowed input arguments.

```cpp
#include <structopt/app.hpp>

struct StyleOptions {
  enum class Color {red, green, blue};

  // e.g., `--color red`
  std::optional<Color> color = Color::red;
};
STRUCTOPT(StyleOptions, color);



int main(int argc, char *argv[]) {

  try {
    auto options = structopt::app("my_app").parse<StyleOptions>(argc, argv);

    // Use parsed argument `options.color`

    if (options.color == StyleOptions::Color::red) {
        std::cout << "#ff0000\n";
    }
    else if (options.color == StyleOptions::Color::blue) {
        std::cout << "#0000ff\n";
    }
    else if (options.color == StyleOptions::Color::green) {
        std::cout << "#00ff00\n";
    }

  } catch (structopt::exception& e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

```console
foo@bar:~$ ./main --color red
#ff0000

foo@bar:~$ ./main -c blue
#0000ff

foo@bar:~$ ./main --color green
#00ff00

foo@bar:~$ ./main -c black
Error: unexpected input `black` provided for enum argument `color`. Allowed values are {red, green, blue}

USAGE: ./my_app [OPTIONS]

OPTIONS:
    -c, --color <color>
```

### Tuple Arguments

Now that we've looked at enum class support, let's build a simple calculator. In this sample, we will use an `std::tuple` to pack all the arguments to the calculator:

```cpp
#include <structopt/app.hpp>

struct CalculatorOptions {

  // types of operations supported
  enum class operation { add, subtract, multiply, divide };

  // single tuple positional argument
  std::tuple<operation, int, int> input;

};
STRUCTOPT(CalculatorOptions, input);



int main(int argc, char *argv[]) {

  try {
    auto options = structopt::app("my_app").parse<CalculatorOptions>(argc, argv);

    auto op = std::get<0>(options.input);
    auto lhs = std::get<1>(options.input);
    auto rhs = std::get<2>(options.input);
    switch(op)
    {
        case CalculatorOptions::operation::add:
            std::cout << lhs + rhs << "\n";
            break;
        case CalculatorOptions::operation::subtract:
            std::cout << lhs - rhs << "\n";
            break;
        case CalculatorOptions::operation::multiply:
            std::cout << lhs * rhs << "\n";
            break;
        case CalculatorOptions::operation::divide:
            std::cout << lhs / rhs << "\n";
            break;
    }
  }
  catch (structopt::exception& e) {
    std::cout << e.what();
    std::cout << e.help();
  }

}
```

```console
foo@bar:~$ ./main add 1 2
3

foo@bar:~$ ./main subtract 5 9
-4

foo@bar:~$ ./main multiply 16 5
80

foo@bar:~$ ./main divide 1331 11
121

foo@bar:~$ ./main add 5
Error: failed to correctly parse tuple `input`. Expected 3 arguments, 2 provided.

USAGE: my_app input

ARGS:
    input
```

### Vector Arguments

`structopt` supports gathering "remaining" arguments at the end of the command, e.g., for use in a compiler:

```bash
$ compiler file1 file2 file3
```

Do this by using an `std::vector<T>` (or other STL containers with `.push_back()`, e.g, `std::deque` or `std::list`).

***NOTE*** Vector arguments have a cardinality of `0..*`, i.e., zero or more arguments. Unlike array types, you can provide zero arguments to a vector and `structopt` will (try to) not complain.

```cpp
#include <structopt/app.hpp>

struct CompilerOptions {
  // Language standard
  // e.g., --std c++17
  std::optional<std::string> std;

  // remaining arguments
  // e.g., ./compiler file1 file2 file3
  std::vector<std::string> files{};
};
STRUCTOPT(CompilerOptions, std, files);



int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("my_app").parse<CompilerOptions>(argc, argv);

    std::cout << "Standard : " << options.std.value_or("not provided") << "\n";
    std::cout << "Files    : { ";
    std::copy(options.files.begin(), options.files.end(),
              std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << "}" << std::endl;
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

***NOTE*** Notice below that the act of gathering remaining arguments is arrested as soon as an optional argument is detected. See the output of `./main file1.cpp file2.cpp --std c++17` below. Notice that `--std=c++17` is not part of the vector. This is because `--std` is a valid optional argument.

```console
foo@bar:~$ ./main
Standard : not provided
Files    : { }

foo@bar:~$ ./main file1.cpp file2.cpp
Standard : not provided
Files    : { file1.cpp file2.cpp }

foo@bar:~$ ./main file1.cpp file2.cpp --std=c++17
Standard : c++17
Files    : { file1.cpp file2.cpp }

foo@bar:~$ ./main --std:c++20 file1.cpp file2.cpp
Standard : c++20
Files    : { file1.cpp file2.cpp }
```

### Compound Arguments

Compound arguments are optional arguments that are combined and provided as a single argument. Example: `ps -aux`

```cpp
#include <structopt/app.hpp>

struct Options {
  // Flag arguments
  std::optional<bool> a = false;
  std::optional<bool> b = false;

  // Optional argument
  // e.g., -c 1.1 2.2
  std::optional<std::array<float, 2>> c = {};
};
STRUCTOPT(Options, a, b, c);



int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("my_app").parse<Options>(argc, argv);

    // Print parsed arguments:

    std::cout << std::boolalpha << "a = " << options.a.value()
              << ", b = " << options.b.value() << "\n";
    if (options.c.has_value()) {
      std::cout << "c = [" << options.c.value()[0] << ", " << options.c.value()[1]
                << "]\n";
    }
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

```console
foo@bar:~$ ./main -ac 3.14 2.718
a = true, b = false
c = [3.14, 2.718]

foo@bar:~$ ./main -ba
a = true, b = true

foo@bar:~$ ./main -c 1.5 3.0 -ab
a = true, b = true
c = [1.5, 3]
```

### Parsing Numbers

#### Integer Literals

`structopt` supports parsing integer literals including hexadecimal, octal, and binary notation.

```cpp
#include <structopt/app.hpp>

struct IntegerLiterals {
  std::vector<int> numbers;
};
STRUCTOPT(IntegerLiterals, numbers);

int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("my_app").parse<IntegerLiterals>(argc, argv);

    for (auto &n : options.numbers)
      std::cout << n << "\n";
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

```console
foo@bar:~$ ./main 1 0x5B 071 0b0101 -35 +98
1
91
57
5
-35
98
```

#### Floating point Literals

As for floating point numbers, `structopt` supports parsing scientific notation (e/E-notation):

```cpp
#include <structopt/app.hpp>

struct FloatLiterals {
  std::vector<float> numbers;
};
STRUCTOPT(FloatLiterals, numbers);

int main(int argc, char *argv[]) {
  try {
    auto options = structopt::app("my_app").parse<FloatLiterals>(argc, argv);

    for (auto &n : options.numbers)
      std::cout << n << "\n";
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

```console
foo@bar:~$ ./main -3.15 +2.717 2E-4 0.1e2 .5 -.3 +5.999
-3.15
2.717
0.0002
10
0.5
-0.3
5.999
```

### Nested Structures

With `structopt`, you can define sub-commands, e.g., `git init args` or `git config [flags] args` using nested structures.

* Simply create a nested structure that inherits from `structopt::sub_command`
* You can use `<nested_struct_object>.has_value()` to check if it has been invoked.

The following program support two sub-commands: `config` and `init`:

```cpp
#include <structopt/app.hpp>

struct Git {
  // Subcommand: git config
  struct Config : structopt::sub_command {
    // flag argument `--global`
    std::optional<bool> global = false;

    // key-value pair, e.g., `user.name "John Doe"`
    std::array<std::string, 2> name_value_pair{};
  };
  Config config;

  // Subcommand: git init
  struct Init : structopt::sub_command {

    // required argument
    // repository name
    std::string name;
  };
  Init init;
};
STRUCTOPT(Git::Config, global, name_value_pair);
STRUCTOPT(Git::Init, name);
STRUCTOPT(Git, config, init);



int main(int argc, char *argv[]) {


  try {
    auto options = structopt::app("my_app").parse<Git>(argc, argv);

    if (options.config.has_value()) {
      // config was invoked
      std::cout << "You invoked `git config`:\n";
      std::cout << "Global : " << std::boolalpha << options.config.global.value() << "\n";
      std::cout << "Input  : (" << options.config.name_value_pair[0] << ", " << options.config.name_value_pair[1] << ")\n";
    }
    else if (options.init.has_value()) {
      // init was invoked
      std::cout << "You invoked `git init`:\n";
      std::cout << "Repository name : " << options.init.name << "\n";
    }


  } catch (structopt::exception& e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

```console
foo@bar:~$ ./main config user.email "john.doe@foo.com"
You invoked `git config`:
Global : false
Input  : (user.email, john.doe@foo.com)

foo@bar:~$ ./main config user.name "John Doe" --global
You invoked `git config`:
Global : true
Input  : (user.name, John Doe)

foo@bar:~$ ./main init my_repo
You invoked `git init`:
Repository name : my_repo



foo@bar:~$ ./main -h

USAGE: my_app [OPTIONS] [SUBCOMMANDS]

OPTIONS:
    -h, --help <help>
    -v, --version <version>

SUBCOMMANDS:
    config
    init




foo@bar:~$ ./main config -h

USAGE: config [FLAGS] [OPTIONS] name_value_pair

FLAGS:
    -g, --global

OPTIONS:
    -h, --help <help>
    -v, --version <version>

ARGS:
    name_value_pair




foo@bar:~$ ./main init -h

USAGE: init [OPTIONS] name

OPTIONS:
    -h, --help <help>
    -v, --version <version>

ARGS:
    name
```

***NOTE*** Notice in the above stdout that the `-h` help option supports printing help both at the top-level struct and at the sub-command level.

***NOTE*** `structopt` does not allow to invoke multiple sub-commands. If one has already been invoked, you will see the following error:

```console
foo@bar:~$ ./main config user.name "John Doe" init my_repo
Error: failed to invoke sub-command `init` because a different sub-command, `config`, has already been invoked.
```

### Sub-Commands, Vector Arguments, and Delimited Positional Arguments

Here's a second example for nested structures with vector arguments and the double dash (`--`) delimiter

```cpp
#include <structopt/app.hpp>

struct CommandOptions {
  struct Sed : structopt::sub_command {
    // --trace
    std::optional<bool> trace = false;

    // remaining args
    std::vector<std::string> args;

    // pattern
    std::string pattern;

    // file
    std::string file;
  };
  Sed sed;
};
STRUCTOPT(CommandOptions::Sed, trace, args, pattern, file);
STRUCTOPT(CommandOptions, sed);



int main(int argc, char *argv[]) {

  auto app = structopt::app("my_app");

  try {

    auto options = app.parse<CommandOptions>(argc, argv);

    if (options.sed.has_value()) {
      // sed has been invoked

      if (options.sed.trace == true) {
        std::cout << "Trace enabled!\n";
      }

      std::cout << "Args    : ";
      for (auto& a : options.sed.args) std::cout << a << " ";
      std::cout << "\n";
      std::cout << "Pattern : " << options.sed.pattern << "\n";
      std::cout << "File    : " << options.sed.file << "\n";
    }
    else {
      std::cout << app.help();
    }

  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

```console
foo@bar:~$ ./main

USAGE: my_app [OPTIONS] [SUBCOMMANDS]

OPTIONS:
    -h, --help <help>
    -v, --version <version>

SUBCOMMANDS:
    sed



foo@bar:~$ ./main sed --trace X=1 Y=2 Z=3 -- 's/foo/bar/g' foo.txt
Trace enabled!
Args    : X=1 Y=2 Z=3
Pattern : s/foo/bar/g
File    : foo.txt
```

### Printing Help

`structopt` will insert two optional arguments for the user: `help` and `version`.

* Using `-h` or `--help` will print the help message and exit.
* Using `-v` or `--version` will print the program version and exit.

```cpp
#include <structopt/app.hpp>

struct Options {
  // positional arguments
  std::string input_file;
  std::string output_file;

  // optional arguments
  std::optional<std::string> bind_address;

  // remaining arguments
  std::vector<std::string> files;
};
STRUCTOPT(Options, input_file, output_file, bind_address, files);



int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app", "1.0.3").parse<Options>(argc, argv);
}
```

```console
foo@bar:~$ ./main -h

USAGE: my_app [OPTIONS] input_file output_file files

OPTIONS:
    -b, --bind-address <bind_address>
    -h, --help <help>
    -v, --version <version>

ARGS:
    input_file
    output_file
    files

foo@bar:~$ ./main -v
1.0.3
```

### Printing CUSTOM Help

`structopt` allows users to provide a custom help messages. Simply pass in your custom help as a string argument to `structopt::app`

```cpp
#include <structopt/app.hpp>

struct Options {
  // positional arguments
  std::string input_file;
  std::string output_file;

  // optional arguments
  std::optional<std::string> bind_address;

  // remaining arguments
  std::vector<std::string> files;
};
STRUCTOPT(Options, input_file, output_file, bind_address, files);

int main(int argc, char *argv[]) {

  try {
    const std::string& custom_help = "Usage: ./my_app input_file output_file [--bind-address BIND_ADDRESS] [files...]\n";
    auto options = structopt::app("my_app", "1.0.3", custom_help).parse<Options>(argc, argv);
  } catch (structopt::exception &e) {
    std::cout << e.what() << "\n";
    std::cout << e.help();
  }
}
```

```console
foo@bar:~$ ./main -h
Usage: ./my_app input_file output_file [--bind-address BIND_ADDRESS] [files...]
```

## Building Samples and Tests

```bash
git clone https://github.com/p-ranav/structopt
cd structopt
mkdir build && cd build
cmake -DSTRUCTOPT_SAMPLES=ON -DSTRUCTOPT_TESTS=ON ..
make
```

### WinLibs + MinGW

For Windows, if you use [WinLibs](http://winlibs.com/) like I do, the cmake command would look like this:

```console
foo@bar:~$ mkdir build && cd build
foo@bar:~$ cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="C:/WinLibs/mingw64/bin/g++.exe" -DSTRUCTOPT_SAMPLES=ON -DSTRUCTOPT_TESTS=ON ..
foo@bar:~$ make

foo@bar:~$ .\tests\structopt_tests.exe
[doctest] doctest version is "2.3.5"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases:     54 |     54 passed |      0 failed |      0 skipped
[doctest] assertions:    393 |    393 passed |      0 failed |
[doctest] Status: SUCCESS!
```

## Compiler Compatibility

* Clang/LLVM >= 5
* MSVC++ >= 14.11 / Visual Studio >= 2017
* Xcode >= 10
* GCC >= 9

## Generating Single Header

```bash
python3 utils/amalgamate/amalgamate.py -c single_include.json -s .
```

## Contributing
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
