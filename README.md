Parse command line arguments by defining a struct

```cpp
#include <structopt/structopt.hpp>

struct Options {
  // positional arguments
  std::string input;
  std::string output;

  // -v or --verbose will set this to true
  std::optional<bool> verbose = false;
};
STRUCTOPT(Options, input_file, output_file, verbose);

// Example usage:
// ./main foo.txt bar.csv --verbose
// ./main -v abc.json def.py
```

Use `structopt::parse<T>` to parse command line arguments into the `Options` struct:

```cpp
int main(int argc, char *argv[]) {
  auto options = structopt::parse<Options>(argc, argv);
  
  if (options.verbose) {
    // enable VERBOSE logging
  }

  // do_something(options.input, options.output);
}
```
