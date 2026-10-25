#include "run.h"
#include "commandline_parser.h"
#include "game_handler.h"
#include "game_saver.h"
#include "helper.h"
#include "json_loader.h"
#include "log.h"
#include "model.h"
#include "request_handler.h"


namespace run {

using namespace std::literals;
namespace net = boost::asio;
// namespace sys = boost::system;
// namespace json = boost::json;


namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
	n = std::max(1u, n);
	std::vector<std::jthread> workers;
	workers.reserve(n - 1);
	// Запускаем n-1 рабочих потоков, выполняющих функцию fn
	while (--n) {
		workers.emplace_back(fn);
	}
	fn();
}


} // namespace


void Run(const start_opt::Args& run_options) {


	const auto address = net::ip::make_address("0.0.0.0"sv);
	constexpr net::ip::port_type port = 8080;

	const unsigned num_threads = std::thread::hardware_concurrency();
	net::io_context ioc(num_threads);

#ifdef MYDEBUG
	net::steady_timer auto_stop_timer{ioc};
	if (run_options.auto_stop_after_sec != 0) {
		auto_stop_timer.expires_after(std::chrono::seconds(run_options.auto_stop_after_sec));
		auto_stop_timer.async_wait([&ioc](boost::system::error_code ec) {
			if (!ec) {
				ioc.stop();
			}
		});
	}
#endif

	std::filesystem::path cfg_path(run_options.cfg_file_path);
	model::Game game = json_loader::LoadGame(cfg_path, cfg_path.filename());

	net::signal_set signals(ioc, SIGINT, SIGTERM);
	signals.async_wait([&ioc /*, game_hndl*/](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
		if (!ec) {
			std::cout << "Signal "sv << signal_number << " received"sv << std::endl;
			ioc.stop();
		} else {
			std::cout << "Error. Signal  "sv << signal_number << " received!"sv << std::endl;
		}
	});

	loot_gen::LootGenerator loot{std::chrono::milliseconds(static_cast<int>(game.GetLootSpawnPeriod() * 1000)),
		game.GetLootSpawnProbability() /*, []() { return helper::GetRandomNum(0.0, 1.0);  }*/};

	auto game_hndl = std::make_shared<game_handler::GameHandler>(
		game, ioc, run_options.tick_period, run_options.is_random_spawn, std::move(loot));


	// SECTION. Save game state to file ===================================================================
	bool do_save_state = !run_options.state_file.empty();
	std::unique_ptr<saver::FileSaver> file_saver;
	if (do_save_state) {
		file_saver = std::make_unique<saver::FileSaver>(*game_hndl, run_options.state_file, ioc);

		// try load existes saved state
		try {
			file_saver->LoadState();
		} catch (const std::invalid_argument& e) {
			srv_log::LogMessage({{"port", port}, {"address", address.to_string()}}, e.what());
		} catch (std::exception& e) {
			std::stringstream ess;
			ess << "Error load state from file '" << e.what() << "'";
			srv_log::LogMessage({{"port", port}, {"address", address.to_string()}}, ess.str());
			throw e;
		}

		if (run_options.save_state_period != 0) {
			// add listener for save game state
			game_hndl->AddListener(
				std::make_unique<saver::FileSerializationListener>(*game_hndl, run_options.state_file, ioc, 3000ms));
		}
	}

	// start game ticks
	if (run_options.tick_period != 0) {
		game_hndl->StartTick();
	}

	auto handler = std::make_shared<http_handler::RequestHandler>(game, game_hndl, run_options.www_root_dir /*, ioc */);

	http_handler::LoggingRequestHandler logging_handler{handler};


	http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, const auto& socket, auto&& send) {
		logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(socket)>(socket),
			std::forward<decltype(send)>(send));
	});

	srv_log::LogMessage({{"port", port}, {"address", address.to_string()}}, "server started"sv);

	RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });

	if (file_saver) {
		file_saver->SaveState();
	}
}

} // namespace run