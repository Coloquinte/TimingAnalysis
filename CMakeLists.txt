
cmake_minimum_required(VERSION 2.8)
project(SQUASH)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fsanitize=address -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

include_directories(${SQUASH_SOURCE_DIR}/include)

set(SOURCES
  src/timing_analysis.cc
)
add_library(squash ${SOURCES})

enable_testing()

set(TEST_LIBS
    squash
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

set(TESTS
    tests/testing.cc
    tests/squash_test.cc
)
add_executable(squash_tests.bin ${TESTS})
target_link_libraries(squash_tests.bin ${TEST_LIBS})
add_test(test squash_tests.bin)

