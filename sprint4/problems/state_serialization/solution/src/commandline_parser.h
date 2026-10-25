#pragma once

#include <boost/program_options.hpp>
#include <optional>
#include <string>

namespace start_opt {

using namespace std::literals;

struct Args {
	size_t tick_period = 0;
	std::string cfg_file_path;
	std::string www_root_dir;
	bool is_random_spawn;
	std::string state_file;
	size_t save_state_period = 0;
	size_t auto_stop_after_sec = 0;
};


[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* argv[]);

} // namespace start_opt