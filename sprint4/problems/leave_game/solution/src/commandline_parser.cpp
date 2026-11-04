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
		("randomize-spawn-points", po::bool_switch(&args.is_random_spawn), "spawn dogs at random positions") //
		("state-file", po::value(&args.state_file)->value_name("path to file with saved state"), "file with saved state of game") //
		("save-state-period", po::value(&args.save_state_period)->value_name("path to file with saved state"), "save period")
		("auto-stop-after", po::value(&args.auto_stop_after_sec)->value_name("auto stop after n second"), "auto stop after n second"); //

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