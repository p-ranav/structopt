Parse command line arguments by defining a struct

```cpp
#include <argo/argo.hpp>

struct Options {
  // positional arguments
  std::string config_file = "config.csv";
  std::string input_file  = "input.txt";
  std::string output_file = "output.txt";

  // optional arguments
  // -v or --verbose
  std::optional<bool> verbose;  
};
ARGO_STRUCT(Opts, config_file, input_file, output_file, verbose);
```

Use `argo::parse<T>` to parse command line arguments into the `Options` struct:

```cpp
int main(int argc, char *argv[]) {
  auto options = argo::parse<Options>(argc, argv);

  // use `options`
}
```
