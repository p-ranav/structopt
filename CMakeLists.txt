cmake_minimum_required(VERSION 3.8)

if(DEFINED PROJECT_NAME)
  set(STRUCTOPT_SUBPROJECT ON)
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
  project(structopt VERSION 0.1.0 LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/p-ranav/structopt"
    DESCRIPTION "Parse command line arguments by defining a struct")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "3.9")
  project(structopt VERSION 0.1.0 LANGUAGES CXX
    DESCRIPTION "Parse command line arguments by defining a struct")
else()
  project(structopt VERSION 0.1.0 LANGUAGES CXX)
endif()

if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
  include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
  conan_basic_setup()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
      add_compile_options("/std:c++latest")
    endif()
  endif()
endif()

option(STRUCTOPT_TESTS "Build structopt tests + enable CTest")
option(STRUCTOPT_SAMPLES "Build structopt samples")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

add_library(structopt INTERFACE)
add_library(structopt::structopt ALIAS structopt)

target_compile_features(structopt INTERFACE cxx_std_17)
target_include_directories(structopt INTERFACE
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

if(STRUCTOPT_SAMPLES)
  add_subdirectory(samples)
endif()

if(STRUCTOPT_TESTS)
  add_subdirectory(tests)
endif()

if(NOT STRUCTOPT_SUBPROJECT)
  configure_package_config_file(structoptConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/structoptConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/structopt)
  write_basic_package_version_file(structoptConfigVersion.cmake
                                   COMPATIBILITY AnyNewerVersion)

  configure_file(structopt.pc.in structopt.pc @ONLY)

  install(TARGETS structopt EXPORT structoptTargets)
  install(EXPORT structoptTargets
          FILE structoptTargets.cmake
          NAMESPACE structopt::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/structopt)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/structoptConfig.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/structoptConfigVersion.cmake
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/structopt)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/structopt.pc
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
  install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/structopt
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
          USE_SOURCE_PERMISSIONS
          PATTERN "*.hpp")
  install(FILES LICENSE LICENSE.magic_enum LICENSE.visit_struct
          DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/licenses/structopt)

  if(EXISTS "${PROJECT_SOURCE_DIR}/.gitignore")
    # Simple glob to regex conversion (.gitignore => CPACK_SOURCE_IGNORE_FILES)
    file(READ ".gitignore" DOT_GITIGNORE)
    string(REPLACE ";" "RANDOMSEQUENCE" DOT_GITIGNORE "${DOT_GITIGNORE}")
    string(REPLACE "\n" ";" DOT_GITIGNORE "${DOT_GITIGNORE}")
    string(REPLACE "RANDOMSEQUENCE" "\\;" DOT_GITIGNORE "${DOT_GITIGNORE}")
    foreach(IGNORE_LINE ${DOT_GITIGNORE})
      if(NOT IGNORE_LINE OR IGNORE_LINE MATCHES "^#")
        continue()
      endif()
      string(REPLACE "\\" "\\\\" IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "." "\\\\." IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "*" ".*" IGNORE_LINE "${IGNORE_LINE}")
      string(REPLACE "+" "\\\\+" IGNORE_LINE "${IGNORE_LINE}")
      list(APPEND CPACK_SOURCE_IGNORE_FILES "${IGNORE_LINE}")
    endforeach()
  endif()

  # extra ignored files
  list(APPEND CPACK_SOURCE_IGNORE_FILES
    .editorconfig
    .git
    .gitignore
    .travis.yml
    .appveyor.yml
  )
  set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
  set(CPACK_GENERATOR "TGZ;TXZ")
  set(CPACK_SOURCE_GENERATOR "TGZ;TXZ")
  include(CPack)
endif()
