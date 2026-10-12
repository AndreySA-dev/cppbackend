#include "commandline_parser.h"

#include <iostream>

namespace start_opt {

std::optional<Args> ParseCommandLine(int argc, const char* argv[]) {

	namespace po = boost::program_options;
	using namespace std::literals;
	Args args;

	po::options_description desc{"Allowed options:"s};
	desc.add_options()("help,h", "produce help message") //
		("tick-period,t", po::value(&args.tick_period)->value_name("milliseconds"s)->default_value(0),
			"set tick period")																		   //
		("config-file,c", po::value(&args.cfg_file_path)->value_name("file"s), "set config file path") //
		("www-root,w", po::value(&args.www_root_dir)->value_name("dir"s), "set static files root")	   //
		// ("randomize-spawn-points", po::value(&args.is_random_spawn)->default_value(false), "spawn dogs at random
		// positions");
		("randomize-spawn-points", po::bool_switch(&args.is_random_spawn), "spawn dogs at random positions"); //

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.contains("help"s)) {
		// Если был указан параметр --help, то выводим справку и возвращаем
		// nullopt
		std::cout << desc;
		return std::nullopt;
	}

	if (!vm.contains("config-file"s)) {
		throw std::runtime_error("Not specified config-file argument"s);
	}
	if (!vm.contains("www-root"s)) {
		throw std::runtime_error("Not specified www-root argument"s);
	}

	return args;
}

} // namespace start_opt