cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(game_model_lib STATIC
	src/model.h
	src/model.cpp
	src/tagged.h
	src/helper.h
	src/helper.cpp
	src/game_handler.h
	src/game_handler.cpp
	src/json_loader.h
	src/json_loader.cpp
	src/loot_generator.h
	src/loot_generator.cpp
	src/codes.h
	src/geom.h
	src/collision_detector.h
	src/collision_detector.cpp
)
target_include_directories(game_model_lib PUBLIC CONAN_PKG::boost)
target_link_libraries(game_model_lib PUBLIC CONAN_PKG::boost Threads::Threads)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(game_model_lib PRIVATE MYDEBUG)
endif()

add_library(game_server_lib STATIC
	src/authenticator.cpp
	src/authenticator.h
	src/http_server.cpp
	src/http_server.h
	src/log.cpp
	src/log.h
	src/perf_timer.h
	src/request_handler.cpp
	src/request_handler.h
	src/sdk.h
	src/token.h
	src/token.cpp
	src/user.cpp
	src/user.h
)
target_include_directories(game_server_lib PUBLIC CONAN_PKG::boost)
target_link_libraries(game_server_lib PUBLIC game_model_lib )
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(game_server_lib PRIVATE MYDEBUG)
endif()

add_executable(game_server 
	src/commandline_parser.cpp
	src/commandline_parser.h
	src/main.cpp
	src/run.cpp
	src/run.h
)
target_include_directories(game_server PRIVATE CONAN_PKG::boost)
target_link_libraries(game_server PRIVATE game_model_lib game_server_lib CONAN_PKG::boost )

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(game_server PRIVATE MYDEBUG)
endif()

add_subdirectory("tests/")
