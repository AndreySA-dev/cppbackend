#include "run.h"
#include "commandline_parser.h"
#include "game_handler.h"
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

	net::signal_set signals(ioc, SIGINT, SIGTERM);
	signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
		if (!ec) {
			std::cout << "Signal "sv << signal_number << " received"sv << std::endl;
			ioc.stop();
		}
	});


	model::Game game = json_loader::LoadGame(run_options.cfg_file_path);
	auto game_hndl =
		std::make_shared<game_handler::GameHandler>(game, ioc, run_options.tick_period, run_options.is_random_spawn);
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
}

} // namespace run