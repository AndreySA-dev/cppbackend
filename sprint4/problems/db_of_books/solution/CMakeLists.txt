cmake_minimum_required(VERSION 3.11)

project(book_manager CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(book_manager 
	src/main.cpp
	src/run.h
	src/run.cpp
)
target_include_directories(book_manager PRIVATE CONAN_PKG::boost CONAN_PKG::libpqxx)
target_link_libraries(book_manager PRIVATE CONAN_PKG::boost CONAN_PKG::libpqxx)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(book_manager PRIVATE MYDEBUG)
endif()

