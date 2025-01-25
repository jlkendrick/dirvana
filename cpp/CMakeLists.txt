cmake_minimum_required(VERSION 3.10)
project(dirvana)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include include/utils)

set(SOURCES
	src/main.cpp

	src/impl/DirectoryCompleter.cpp
	src/impl/RecentlyAccessedCache.cpp

	src/impl/utils/DoublyLinkedList.cpp
	src/impl/utils/Node.cpp
)

add_executable(dirvana ${SOURCES})