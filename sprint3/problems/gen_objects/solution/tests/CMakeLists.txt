
include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)

add_executable(game_server_tests 
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

target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::boost)