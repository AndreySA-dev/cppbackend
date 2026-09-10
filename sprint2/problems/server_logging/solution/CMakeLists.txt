cmake_minimum_required(VERSION 3.11)

project(server_logging CXX)
set(CMAKE_CXX_STANDARD 20)

# обратите внимание на аргумент TARGETS у команды conan_basic_setup
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# find_package(Boost 1.78.0 REQUIRED)
# if(Boost_FOUND)
#   include_directories(${Boost_INCLUDE_DIRS})
# endif()

# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)

add_executable(server_logging
	src/main.cpp
	src/http_server.cpp
	src/http_server.h
	src/sdk.h
	src/model.h
	src/model.cpp
	src/tagged.h
	src/boost_json.cpp
	src/json_loader.h
	src/json_loader.cpp
	src/request_handler.cpp
	src/request_handler.h
	src/helper.cpp
	src/helper.h
	src/log.h
	src/log.cpp
)
# target_link_libraries(server_logging PRIVATE Threads::Threads)

# используем "импортированную" цель CONAN_PKG::boost
target_include_directories(server_logging PRIVATE CONAN_PKG::boost)
target_link_libraries(server_logging CONAN_PKG::boost) 
