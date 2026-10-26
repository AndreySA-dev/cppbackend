cmake_minimum_required(VERSION 3.11)

project(db_books CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(db_books 
	src/main.cpp
)
target_include_directories(db_books PRIVATE CONAN_PKG::boost)
target_link_libraries(db_books PRIVATE CONAN_PKG::boost )
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(db_books PRIVATE MYDEBUG)
endif()

