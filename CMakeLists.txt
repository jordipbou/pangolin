cmake_minimum_required (VERSION 3.8)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_FLAGS "-ansi -pedantic -O0")

project(pangolin LANGUAGES C)

set(pangolin_VERSION_MAJOR 0)
set(pangolin_VERSION_MINOR 1)

add_executable (
	tests
	src/tests.c
	deps/Unity/src/unity.c)

target_include_directories (tests PRIVATE src deps/Unity/src)

add_executable (
	repl
	src/repl.c)

target_include_directories (repl PRIVATE src)

add_executable (
  data_tests
	src/data_tests.c
	deps/Unity/src/unity.c)

target_include_directories (data_tests PRIVATE src deps/Unity/src)
