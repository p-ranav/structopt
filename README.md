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
 
   // You want a flag?
   // Use `std::optional<bool>` and provide a default value. 
   //   e.g., -v
   //   e.g., --verbose
   std::optional<bool> verbose = false;

   // Directly define and use enum classes
   // The argument (string) will be converted (if possible)
   // into the equivalent enum value
   //   e.g., --log-level debug
   //   e.g., -l error
   enum class LogLevel { debug, info, warn, error, critical };
   std::optional<LogLevel> log_level = LogLevel::info;

   // Here, structopt will check for `-u` or `--user` 
   // and parse the next 2 arguments into an `std::pair`
   std::optional<std::pair<std::string, std::string>> user;

   // You can use containers like std::vector
   // when you don't know the total number of arguments
   // but want to collect them all into a list
   std::vector<std::string> files;
};
STRUCTOPT(Options, config_file, bind_address, verbose, log_level, user, files);
```

Create a `structopt::app` and parse the command ling arguments into the `Options` struct:

```cpp
int main(int argc, char *argv[]) {
  
  // Line of code that does all the work:
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  // Print out parsed arguments:

  // std::cout << "config_file  = " << options.config_file << "\n";
  // std::cout << "bind_address = " << options.bind_address.value_or("not provided") << "\n";
  // std::cout << "verbose      = " << std::boolalpha << options.verbose.value() << "\n";
  // ...
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

▶ ./my_app config_2.csv --bind-address 192.168.7.3 -log-level debug file1.txt file3.txt file4.txt --user "Pranav Kumar" "pranav.kumar@foo.com"
config_file  = config_2.csv
bind_address = 192.168.7.3
verbose      = false
log_level    = 0
user         = Pranav Kumar<pranav.kumar@foo.com>
files        = { file1.txt file3.txt file4.txt }
```

## Table of Contents

*    Getting Started[#getting-started]
     *    [Positional Arguments](#positional-arguments)
     *    [Optional Arguments](#optional-arguments)
*    [Building Samples](#building-samples)
*    [Generating Single Header](#generating-single-header)
*    [Contributing](#contributing)
*    [License](#license)

## Getting Started

Include `<structopt/app.hpp>` and you're good to go.

`structopt` supports a variety of argument types including positional, optional, flags, and compound arguments. Below you can see how to configure each of these types:

### Positional Arguments

Here's an example of some positional arguments:

```cpp
#include <structopt/app.hpp>

struct FileOptions {
  std::string input_file;
  std::string output_file;
};
STRUCTOPT(FileOptions, input_file, output_file);

int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<FileOptions>(argc, argv);

  std::cout << "\nInput file  : " << options.input_file << "\n";
  std::cout << "Output file : " << options.output_file << "\n";
}
```

Now we can run our program like so:

```bash
▶ ./main foo.txt bar.csv

Input file  : foo.txt
Output file : bar.csv
```

### Optional Arguments

Now, let's look at optional arguments. To configure an optional argument, use `std::optional` in the options struct:

```cpp
#include <structopt/app.hpp>

struct Options {
  std::optional<std::array<float, 3>> fixed_point;
};
STRUCTOPT(Options, fixed_point);
```

The above optional argument can be provided as `-f`, or `--fixed_point` or `--fixed-point`.

```cpp
int main(int argc, char *argv[]) {
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  if (options.fixed_point.has_value()) {
    std::cout << "Point: {" << options.fixed_point.value()[0] << ", " 
                            << options.fixed_point.value()[1] << ", "
                            << options.fixed_point.value()[2] << "}\n";
  }
}
```

Now we can run our program like so:

```bash
▶ ./main

▶ ./main --fixed_point 1.1 2.2 3.3
Point: {1.1, 2.2, 3.3}

▶ ./main -f 1 2 3
Point: {1, 2, 3}
```

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
