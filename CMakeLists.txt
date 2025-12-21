cmake_minimum_required(VERSION 3.16)
project(dirvana LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_C_COMPILER /usr/bin/clang)

# Build type optimization
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Size optimization flags
set(SIZE_COMPILE_FLAGS
    -Os                    # Optimize for size
)

if(APPLE)
    set(SIZE_LINK_FLAGS
        -Wl,-dead_strip
        -Wl,-x
    )
else()
    set(SIZE_LINK_FLAGS
        -Wl,--gc-sections
        -Wl,--strip-all
        -Wl,--as-needed
    )
endif()

set(COMMON_COMPILE_FLAGS 
    -Wall -Wextra -Wpedantic -Werror 
    -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function 
    -Wno-missing-field-initializers -Wno-deprecated-declarations
    ${SIZE_COMPILE_FLAGS}
)


# ======== SQLite3 Configuration ========
add_library(sqlite3_lib STATIC src/impl/lib/sqlite3.c)
set_source_files_properties(src/impl/lib/sqlite3.c PROPERTIES LANGUAGE C COMPILE_FLAGS "-w -Os")
set_target_properties(sqlite3_lib PROPERTIES LINKER_LANGUAGE C)
target_compile_definitions(sqlite3_lib PRIVATE 
    SQLITE_ENABLE_RTREE
    SQLITE_ENABLE_FTS5
    SQLITE_ENABLE_JSON1
    SQLITE_THREADSAFE=1
    SQLITE_DEFAULT_MEMSTATUS=0    # Disable memory status tracking
    SQLITE_OMIT_DEPRECATED=1      # Remove deprecated features
    SQLITE_OMIT_LOAD_EXTENSION=1  # Disable extension loading
    SQLITE_OMIT_TRACE=1           # Disable tracing
    SQLITE_OMIT_UTF16=1           # Disable UTF-16 support
)
target_include_directories(sqlite3_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/lib)
if(APPLE)
    target_link_libraries(sqlite3_lib "-framework CoreFoundation" "-framework Security")
endif()


# ======== Application Code Configuration ========
set(DIRVANA_SOURCES
	src/impl/Config.cpp
	src/impl/Database.cpp
	src/impl/Handler.cpp
	src/impl/tables/Table.cpp
	src/impl/utils/Helpers.cpp
 	src/impl/utils/Types.cpp
)
add_library(dirvana_lib ${DIRVANA_SOURCES})
target_include_directories(dirvana_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/utils
)
target_include_directories(dirvana_lib SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lib
	${CMAKE_CURRENT_SOURCE_DIR}/include/lib/sqlite_modern_cpp/hdr
)
target_compile_options(dirvana_lib PRIVATE ${COMMON_COMPILE_FLAGS})

# -------- SQLite3 Configuration for dirvana_lib --------
target_include_directories(sqlite3_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lib
)
target_link_libraries(dirvana_lib sqlite3_lib)

# ======== Application Binary Configuration ========
add_executable(dv-binary src/main.cpp)
target_link_libraries(dv-binary dirvana_lib)

# Additional size optimizations for the main binary
target_compile_options(dv-binary PRIVATE ${SIZE_COMPILE_FLAGS})
target_link_options(dv-binary PRIVATE ${SIZE_LINK_FLAGS})
set_target_properties(dv-binary PROPERTIES
    OUTPUT_NAME "dv-binary"
)

# ======== Tests Configuration ========
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
set(TEST_SOURCES
    tests/test_Database.cpp
)
add_executable(test ${TEST_SOURCES})
target_link_libraries(test dirvana_lib GTest::GTest GTest::Main pthread)