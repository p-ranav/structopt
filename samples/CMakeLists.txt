
add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE structopt::structopt)

add_executable(positional_arguments positional_arguments.cpp)
target_link_libraries(positional_arguments PRIVATE structopt::structopt)

add_executable(optional_arguments optional_arguments.cpp)
target_link_libraries(optional_arguments PRIVATE structopt::structopt)

add_executable(flag_arguments flag_arguments.cpp)
target_link_libraries(flag_arguments PRIVATE structopt::structopt)
