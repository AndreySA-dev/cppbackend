
include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)

add_executable(game_server_tests 
	./helper_tests.cpp
	./loot_generator_tests.cpp
)

target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2)
target_link_libraries(game_server_tests PRIVATE game_model_lib CONAN_PKG::boost )