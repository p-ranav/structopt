Parse command line arguments by defining a struct

```cpp
#include <structopt/structopt.hpp>
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

   // Use `std::optional<bool>` and provide a default value. 
   // Now you have a flag!
   //   e.g., -v
   //   e.g., --verbose
   // Passing this flag will set this 
   // value to (!default_value), i.e., true
   std::optional<bool> verbose = false;

   // structopt also support defining enum classes
   // The argument (string) will be converted (if possible)
   // into the equivalent enum value
   //   e.g., --log-level debug
   //   e.g., -l error
   enum class LogLevel { debug, info, warn, error, critical };
   std::optional<LogLevel> log_level = LogLevel::info;

   // You can use containers like std::vector
   // to save variadic arguments of some type
   std::vector<std::string> files;
};
STRUCTOPT(Options, config_file, bind_address, verbose, log_level, files);
```

Simply call `structopt::parse<T>` to parse command line arguments into the `Options` struct:

```cpp
int main(int argc, char *argv[]) {
  auto options = structopt::parse<Options>(argc, argv);

  // Print out parsed arguments:

  std::cout << "config_file  = " << options.config_file << "\n";
  std::cout << "bind_address = " << options.bind_address.value_or("not provided") << "\n";
  std::cout << "verbose      = " << std::boolalpha << options.verbose.value() << "\n";
  std::cout << "log_level    = " << static_cast<int>(options.log_level.value()) << "\n";
  std::cout << "files        = {";
  std::copy(options.files.begin(), options.files.end(), std::ostream_iterator<std::string>(std::cout, " "));
  std::cout << "}" << std::endl; 
}
```
