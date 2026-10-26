#include "run.h"
#include <iostream>

using namespace std::literals;

int main(int argc, const char* argv[]) {
	// std::cerr << "Start.." << std::endl;
	try {
		// if (argc == 1) {
		// 	std::cout << "Usage: db_example <conn-string>\n"sv;
		// 	return EXIT_SUCCESS;
		// } else if (argc != 2) {
		// 	std::cerr << "Invalid command line\n"sv;
		// 	return EXIT_FAILURE;
		// }

		Run(argv[1]);


	} catch (const std::exception& e) {
	// 	std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	// std::cerr << "End.." << std::endl;
}