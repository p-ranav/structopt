cmake_minimum_required(VERSION 3.6)
project(structopt)

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Disable deprecation for windows
if (WIN32)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# structopt executable
file(GLOB structopt_TEST_SOURCES
    main.cpp
    test_single_positional_argument.cpp
    test_array_positional_argument.cpp
    test_3d_array_positional_argument.cpp
    test_single_optional_argument.cpp
    test_array_optional_argument.cpp
    test_single_positional_and_optional.cpp
    test_nested_struct.cpp
    test_multiple_nested_structs_positional.cpp
    test_vector_positional_argument.cpp
    test_vector_optional_argument.cpp
    test_optional_flag_argument.cpp
    test_enum_positional_argument.cpp
    test_enum_optional_argument.cpp
    test_optional_argument_naming.cpp
    test_double_dash_option_delimiter.cpp
    test_trailing_positional_arguments.cpp
    test_tuple_positional_argument.cpp
    test_tuple_optional_argument.cpp
    test_deque_positional_argument.cpp
    test_deque_optional_argument.cpp
)
set_source_files_properties(main.cpp
    PROPERTIES
    COMPILE_DEFINITIONS DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
ADD_EXECUTABLE(structopt ${structopt_TEST_SOURCES})
INCLUDE_DIRECTORIES("../include" ".")
set_target_properties(structopt PROPERTIES OUTPUT_NAME tests)
set_property(TARGET structopt PROPERTY CXX_STANDARD 17)

# Set ${PROJECT_NAME} as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT structopt)
