Parse command line arguments by defining a struct

```cpp
#include <argo/argo.hpp>

struct Options {
  // positional arguments
  std::string input  = "input.txt";
  std::string output = "output.csv";

  // optional arguments
  // -v or --verbose
  std::optional<bool> verbose;
};
ARGO_STRUCT(Options, input_file, output_file, verbose);
```

Use `argo::parse<T>` to parse command line arguments into the `Options` struct:

```cpp
int main(int argc, char *argv[]) {
  auto options = argo::parse<Options>(argc, argv);
  
  if (options.verbose) {
    // enable VERBOSE logging
  }

  // do_something(options.input, options.output);
}
```
