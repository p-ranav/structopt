<p align="center">
  <img height="70" src="img/logo.png"/>  
</p>

<p align="center">
  Parse command line arguments by defining a struct
</p>

<p align="center">
  <a href="https://en.wikipedia.org/wiki/C%2B%2B17">
    <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="standard"/>
  </a>
  <a href="https://github.com/p-ranav/tabulate/blob/master/LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="license"/>
  </a>
  <img src="https://img.shields.io/badge/version-1.0-blue.svg?cacheSeconds=2592000" alt="version"/>
</p>

## Quick Start

```cpp
struct Options {
   // positional argument
   //   e.g., ./main <file>
   std::string config_file;

   // optional argument
   //   e.g., -b "192.168.5.3"
   //   e.g., --bind_address "192.168.5.3"
   // The long option can be passed in kebab case
   //   e.g., --bind-address "192.168.5.3"
   std::optional<std::string> bind_address;
 
   // flag argument
   // Use `std::optional<bool>` and provide a default value. 
   //   e.g., -v
   //   e.g., --verbose
   std::optional<bool> verbose = false;

   // Directly define and use enum classes
   //   e.g., --log-level debug
   //   e.g., -l error
   enum class LogLevel { debug, info, warn, error, critical };
   std::optional<LogLevel> log_level = LogLevel::info;

   // Here, structopt will check for `-u` or `--user` 
   // and parse the next 2 arguments into an `std::pair`
   std::optional<std::pair<std::string, std::string>> user;

   // Use containers like std::vector
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

```bash
▶ ./my_app config.csv file5.csv file6.json
config_file  = config.csv
bind_address = not provided
verbose      = false
log_level    = 1
user         = not provided
files        = { file5.csv file6.json }

▶ ./my_app config.csv --bind-address localhost:9000 -v -log-level error file1.txt file2.txt
config_file  = config.csv
bind_address = localhost:9000
verbose      = true
log_level    = 3
user         = not provided
files        = { file1.txt file2.txt }

▶ ./my_app config_2.csv --bind-address 192.168.7.3 -log-level debug file1.txt file3.txt file4.txt --user "John Doe" "john.doe@foo.com"
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
          *    [Combining Positional and Optional Arguments](#combining-positional-and-optional-arguments)
     *    [Flag Arguments](#flag-arguments)
     *    [Enum Class Arguments (Choices)](#enum-class-arguments)
     *    [Tuple Arguments](#tuple-arguments)
     *    [Vector Arguments](#vector-arguments)
     *    [Compound Arguments](#compound-arguments)
          *    [Positional and Compound Toggle Arguments](#positional-and-compound-toggle-arguments)
     *    [Parsing Numbers](#parsing-numbers)
          *    [Integer Literals](#integer-literals)
          *    [Floating point Literals](#floating-point-literals)
     *    [Nested Structures (Sub-commands)](#nested-structures)
     *    [Printing Help](#printing-help)
*    [Building Samples](#building-samples)
*    [Generating Single Header](#generating-single-header)
*    [Contributing](#contributing)
*    [License](#license)

## Getting Started 

Include `<structopt/app.hpp>` and you're good to go.

`structopt` supports a variety of argument types including positional, optional, flags, and compound arguments. Below you can see how to configure each of these types:

### Positional Arguments

Here's an example of two positional arguments: `input_file` and `output_file`. `input_file` is expected to be the first argument and `output_file` is expected to be the second argument

```cpp
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

```bash
▶ ./main foo.txt bar.csv

Input file  : foo.txt
Output file : bar.csv

▶ ./main foo.csv
Error: expected value for positional argument `output_file`.

USAGE: ./my_app input_file output_file

ARGS:
    input_file
    output_file
```

### Optional Arguments

Now, let's look at optional arguments. To configure an optional argument, use `std::optional` in the options struct:

```cpp
struct Options {
  // Optional argument
  // -f, --fixed_point, --fixed-point <fixed_point...>
  std::optional<std::array<float, 3>> fixed_point;
};
STRUCTOPT(Options, fixed_point);



int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  if (options.fixed_point.has_value()) {
    std::cout << "Point: {" << options.fixed_point.value()[0] << ", " 
                            << options.fixed_point.value()[1] << ", "
                            << options.fixed_point.value()[2] << "}\n";
  }
}
```

```bash
▶ ./main

▶ ./main --fixed_point 1.1 -2.2 3.3
Point: {1.1, -2.2, 3.3}

▶ ./main -f 1 2 3
Point: {1, 2, 3}
```

#### Double dash (`--`) Argument

A double dash (`--`) is used in most bash built-in commands and many other commands to signify the end of command options, after which only positional parameters are accepted.

Example use: lets say you want to `grep` a file for the string `-v` - normally `-v` will be considered the option to reverse the matching meaning (only show lines that do not match), but with `--` you can `grep` for string `-v` like this:

```cpp
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

```bash
▶ ./main -v foo bar.txt
`-v` provided - Matching is now reversed
Search   : foo
Pathspec : bar.txt

▶ ./main -- -v bar.txt
Search   : -v
Pathspec : bar.txt
```

#### Combining Positional and Optional Arguments

```cpp
struct Option {
  // positional argument
  // input number
  int input{0};

  // flag argument
  // enable verbosity
  std::optional<bool> verbose = false;
};
STRUCTOPT(Option, input, verbose);



int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Option>(argc, argv);

  if (options.verbose == true) {
    std::cout << "The square of " << options.input << " is " << (options.input * options.input) << "\n";
  } else {
    std::cout << options.input * options.input << "\n";
  }
}
```

```bash
▶ ./main 3
9

▶ ./main 5 -v
The square of 5 is 25

▶ ./main --verbose 5
The square of 4 is 16
```

### Flag Arguments

Flag arguments are `std::optional<bool>` with a default value. 

***NOTE*** The default value here is important. It is not a flag if a default value isn't provided. It will simply be an optional argument. 

***NOTE*** If `--verbose` is a flag argument with a default value of `false`, then providing the argument will set it to `true`. If `--verbose` does not have a default value, then `structopt` will expect the user to provide a value, e.g., `--verbose true`. 

```cpp
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

```bash
▶ ./main

▶ ./main -v
Verbosity enabled

▶ ./main --verbose
Verbosity enabled
```

### Enum Class Arguments

Thanks to [magic_enum](https://github.com/Neargye/magic_enum), `structopt` supports enum classes. You can use an enum classes to ask the user to provide a value given a choice of values, restricting the possible set of allowed input arguments.

```cpp
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

```bash
▶ ./main --color red
#ff0000

▶ ./main -c blue
#0000ff

▶ ./main --color green
#00ff00

▶ ./main -c black
Error: unexpected input `black` provided for enum argument `color`. Allowed values are {red, green, blue}

USAGE: ./my_app [OPTIONS]

OPTIONS:
    -c, --color <color>
```

### Tuple Arguments

Now that we've looked at enum class support, let's build a simple calculator. In this sample, we will use an `std::tuple` to pack all the arguments to the calculator:

```cpp
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

```bash
▶ ./main add 1 2
3

▶ ./main subtract 5 9
-4

▶ ./main multiply 16 5
80

▶ ./main divide 1331 11
121

▶ ./main add 5
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
  auto options = structopt::app("my_app").parse<CompilerOptions>(argc, argv);

  // Some code to print parsed arguments
}
```

***NOTE*** Notice below that the act of gathering remaining arguments is arrested as soon as an optional argument is detected. See the output of `./main file1.cpp file2.cpp --std c++17` below. Notice that `--std` and `c++17` are not part of the vector. This is because `--std` is a valid optional argument.

```bash
▶ ./main
Standard : not provided
Files    : { }

▶ ./main file1.cpp file2.cpp
Standard : not provided
Files    : { file1.cpp file2.cpp }

▶ ./main file1.cpp file2.cpp --std c++17
Standard : c++17
Files    : { file1.cpp file2.cpp }

▶ ./main --std c++20 file1.cpp file2.cpp
Standard : c++20
Files    : { file1.cpp file2.cpp }
```

### Compound Arguments

Compound arguments are optional arguments that are combined and provided as a single argument. Example: `ps -aux`

```cpp
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
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  // Some code to print parsed arguments
}
```

```bash
▶ ./main -ac 3.14 2.718
a = true, b = false
c = [3.14, 2.718]

▶ ./main -ba
a = true, b = true

▶ ./main -c 1.5 3.0 -ab
a = true, b = true
c = [1.5, 3]
```

#### Positional and Compound Toggle Arguments

```cpp
struct Options {
  // Positional arguments
  std::array<int, 3> numbers = {0, 0, 0};

  // Flag arguments
  std::optional<bool> a = false;
  std::optional<bool> b = false;

  // Optional argument
  // e.g., -c 1.1 2.2
  std::optional<std::array<float, 2>> c = {};

  // Remaining arguments
  std::optional<std::vector<std::string>> files;
};
STRUCTOPT(Options, numbers, a, b, c, files);


int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  // Some code to print parsed arguments
}
```

```bash
▶ ./main 1 2 3 -abc 3.14 2.718 --files a.txt b.txt c.txt
numbers = [1, 2, 3]
a = true, b = true
c = [3.14, 2.718]
files = a.txt b.txt c.txt
```

### Parsing Numbers

#### Integer Literals

`structopt` supports parsing integer literals including hexadecimal, octal, and binary notation. 

```cpp
struct IntegerLiterals {
  std::vector<int> numbers;
};
STRUCTOPT(IntegerLiterals, numbers);

// main() omitted for brevity
```

```bash
▶ ./main 1 0x5B 071 0b0101 -35 +98
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
struct FloatLiterals {
  std::vector<float> numbers;
};
STRUCTOPT(FloatLiterals, numbers);

// main() omitted for brevity
```

```bash
▶ ./main -3.15 +2.717 2E-4 0.1e2 .5 -.3 +5.999
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

```bash
▶ ./main config user.email "john.doe@foo.com"
You invoked `git config`:
Global : false
Input  : (user.email, john.doe@foo.com)

▶ ./main config user.name "John Doe" --global
You invoked `git config`:
Global : true
Input  : (user.name, John Doe)

▶ ./main init my_repo
You invoked `git init`:
Repository name : my_repo

▶ ./main config -h

USAGE: config [FLAGS] [OPTIONS] name_value_pair

FLAGS:
    -g, --global

OPTIONS:
    -h, --help <help>
    -v, --version <version>

ARGS:
    name_value_pair

▶ ./main init -h

USAGE: init [OPTIONS] name

OPTIONS:
    -h, --help <help>
    -v, --version <version>

ARGS:
    name
```

***NOTE*** `structopt` does not allow to invoke multiple sub-commands. If one has already been invoked, you will see the following error:

```bash
▶ ./main config user.name "John Doe" init my_repo
Error: failed to invoke sub-command `init` because a different sub-command, `config`, has already been invoked.
```

### Printing Help

`structopt` will insert two optional arguments for the user: `help` and `version`. 

* Using `-h` or `--help` will print the help message and exit.
* Using `-v` or `--version` will print the program version and exit. 

```cpp
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

```bash
▶ ./main -h

USAGE: my_app [OPTIONS] input_file output_file files

OPTIONS:
    -b, --bind-address <bind_address>
    -h, --help <help>
    -v, --version <version>

ARGS:
    input_file
    output_file
    files

▶ ./main -v
1.0.3
```

***NOTE*** Admittedly, the above help message doesn't look great; none of the arguments have a description - something that is configurable in other argument parsers. `structopt` does its best to infer details about arguments from the user-defined struct including argument name, data type, and argument type. Unforunately, `structopt` (for now) does not provide any API to the user to configure (e.g., by providing a map) documentation for each of the fields in the struct. 

## Building Samples

```bash
git clone https://github.com/p-ranav/structopt
cd structopt
mkdir build && cd build
cmake -DSTRUCTOPT_SAMPLES=ON -DSTRUCTOPT_TESTS=ON ..
make
```

## Generating Single Header

```bash
python3 utils/amalgamate/amalgamate.py -c single_include.json -s .
```

## Contributing
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
