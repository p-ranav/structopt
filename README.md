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

Simply call `structopt::parse<T>` to parse command line arguments into the `Options` struct:

```cpp
int main(int argc, char *argv[]) {
  
  // Line of code that does all the work:
  auto options = structopt::app("my_app").parse<Options>(argc, argv);

  // Print out parsed arguments:

  std::cout << "config_file  = " << options.config_file << "\n";
  std::cout << "bind_address = " << options.bind_address.value_or("not provided") << "\n";
  std::cout << "verbose      = " << std::boolalpha << options.verbose.value() << "\n";
  std::cout << "log_level    = " << static_cast<int>(options.log_level.value()) << "\n";
  if (options.user.has_value())
    std::cout << "user         = " << options.user.value().first << "<" << options.user.value().second << ">\n";
  else
    std::cout << "user         = " << "not provided\n";
  std::cout << "files        = { ";
  std::copy(options.files.begin(), options.files.end(), std::ostream_iterator<std::string>(std::cout, " "));
  std::cout << "}" << std::endl; 
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

## Generating Single Header

```bash
python3 utils/amalgamate/amalgamate.py -c single_include.json -s .
```

## Contributing
Contributions are welcome, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
The project is available under the [MIT](https://opensource.org/licenses/MIT) license.
