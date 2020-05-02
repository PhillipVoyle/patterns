
project(patterns)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(patterns INTERFACE)
target_include_directories(patterns INTERFACE include/)

add_subdirectory(tests)
