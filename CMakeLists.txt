cmake_minimum_required(VERSION 3.10)
project(dirvana)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

# Define include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/include/caches
)

# Define source files
set(LIB_SOURCES
    src/impl/DirectoryCompleter.cpp
    src/impl/PathMap.cpp
    
    src/impl/caches/RecentlyAccessedCache.cpp
    # src/impl/caches/FrequencyBasedCache.cpp
    
    src/impl/utils/DoublyLinkedList.cpp
    # src/impl/utils/DLLTraverser.cpp
)

# Create a library for shared application code
add_library(dirvana_lib ${LIB_SOURCES})

# Create the main executable
add_executable(dv-binary src/main.cpp)
target_link_libraries(dv-binary dirvana_lib)

# Find and configure Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# # Define test source files
# set(TEST_SOURCES
#     tests/test_DirectoryCompleter.cpp
#     tests/test_PathMap.cpp

#     tests/test_RecentlyAccessedCache.cpp
#     # tests/test_FrequencyBasedCache.cpp

#     # tests/test_DoublyLinkedList.cpp
# )

# # Create the test executable
# add_executable(test ${TEST_SOURCES})
# target_link_libraries(test dirvana_lib GTest::GTest GTest::Main pthread)