#include <doctest.hpp>
#include <structopt/app.hpp>

using doctest::test_suite;

struct FileOptions {
  // Positional arguments
  // ./main <input_file> <output_file>
  std::string input_file;
  std::string output_file;
};
STRUCTOPT(FileOptions, input_file, output_file);

TEST_CASE("structopt reports error on excess positional arguments" * test_suite("single_optional")) {
  {
    bool exception_thrown{false};
    try {
      auto arguments = structopt::app("test").parse<FileOptions>(std::vector<std::string>{"./main", "file1", "file2", "file3"});
    } catch(structopt::exception& e) {
      exception_thrown = true;
    }
    REQUIRE(exception_thrown == true);
  }
}