cmake_minimum_required(VERSION 3.11)

project(game_server_tests CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

add_executable(tests 
	../src/helper.h
	../src/helper.cpp
	../src/loot_generator.h
	../src/loot_generator.cpp
	../src/helper.cpp
	./helper_tests.cpp
	./loot_generator_tests.cpp
	../src/model.h
	../src/model.cpp
	../src/json_loader.h
	../src/json_loader.cpp

)

target_link_libraries(tests PRIVATE CONAN_PKG::catch2)
target_link_libraries(tests PRIVATE CONAN_PKG::boost)