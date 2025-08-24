cmake_minimum_required(VERSION 3.14)
project(simple_shell C CXX)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# include/ is where *.h live
include_directories(${PROJECT_SOURCE_DIR}/include)

# our shell executable
add_executable(shell
  src/main.c
  src/parser.c
  src/builtin.c
  src/history.c
)

include(FetchContent)
FetchContent_Declare(
  googletest
  DOWNLOAD_EXTRACT_TIMESTAMP false
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(shell_test
  gtest/shell_test.cpp
)

target_link_libraries(shell_test GTest::gtest_main)
add_dependencies(shell_test shell)


include(GoogleTest)
gtest_discover_tests(shell_test)
