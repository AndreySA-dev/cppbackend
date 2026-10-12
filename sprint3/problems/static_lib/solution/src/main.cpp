#include "sdk.h"
#include "log.h"
#include "commandline_parser.h"
#include "run.h"

#include <iostream>

using namespace std::literals;


int main(int argc, const char* argv[]) {

	srv_log::InitBoostLogFilter(); // initialize logging

	try {

		auto start_options = start_opt::ParseCommandLine(argc, argv);

		if (start_options) {
			run::Run(*start_options);
		}


	} catch (const std::exception& ex) {

		std::cerr << ex.what() << std::endl;
		srv_log::LogMessage({{"code", EXIT_FAILURE}, {"exception", ex.what()}}, "server exited"sv);
		return EXIT_FAILURE;
	}

	srv_log::LogMessage({{"code", 0}}, "server exited"sv);
}
