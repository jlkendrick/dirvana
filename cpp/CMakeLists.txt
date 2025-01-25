cmake_minimum_required(VERSION 3.10)
project(dirvana)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

# Define include directories
include_directories(include include/utils)

# Define source files
set(LIB_SOURCES
    src/impl/DirectoryCompleter.cpp
    src/impl/PathMap.cpp
    src/impl/RecentlyAccessedCache.cpp
    src/impl/utils/DoublyLinkedList.cpp
)

# Create a library for shared application code
add_library(dirvana_lib ${LIB_SOURCES})

# Create the main executable
add_executable(main src/main.cpp)
target_link_libraries(main dirvana_lib)

# Find and configure Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Define test source files
set(TEST_SOURCES
    tests/test_DirectoryCompleter.cpp
)

# Create the test executable
add_executable(test ${TEST_SOURCES})
target_link_libraries(test dirvana_lib GTest::GTest GTest::Main pthread)