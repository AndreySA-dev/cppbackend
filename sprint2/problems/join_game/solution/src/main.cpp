// curl -i -d "{\"userName\": \"Mikki\", \"mapId\": \"map1\"}" -H "Content-Type: application/json" -X POST "http://192.168.1.205:8080/api/v1/game/join"
// curl -i -H "Authorization: Bearer 6516861d89ebfff147bf2eb2b5153ae1" -X GET "http://192.168.1.205:8080/api/v1/game/players"

#include "sdk.h"
#include "json_loader.h"
#include "log.h"
#include "request_handler.h"
#include "game_handler.h"


#include <filesystem>
#include <iostream>
#include <thread>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace json = boost::json;


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

int main(int argc, const char* argv[]) {

	srv_log::InitBoostLogFilter(); // initialize logging

	if (argc != 3) {
		std::cerr << "Usage: game_server <game-config-json> <wwwroot directory>"sv << std::endl;
		return EXIT_FAILURE;
	}

	if (!std::filesystem::exists(argv[2])) {
		std::cerr << "wwwroot directory is not found."sv << std::endl;
		return EXIT_FAILURE;
	}
	
	try {
		
		// 1. Загружаем карту из файла и построить модель игры
		model::Game game = json_loader::LoadGame(argv[1]);
		
		const auto address = net::ip::make_address("0.0.0.0");
		constexpr net::ip::port_type port = 8080;
				
		// 2. Инициализируем io_context
		const unsigned num_threads = std::thread::hardware_concurrency();
		net::io_context ioc(num_threads);
		
		// 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
		net::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
			if (!ec) {
				std::cout << "Signal "sv << signal_number << " received"sv << std::endl;
				ioc.stop();
			}
		});
		

		game_handler::GameHandler game_handler(game);
		
		// 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
		std::string wwwroot_path = argv[2];
		// http_handler::RequestHandler handler{game, game_handler, wwwroot_path, ioc};
		auto handler = std::make_shared<http_handler::RequestHandler>(game, game_handler, wwwroot_path, ioc);

		// Оборачиваем его в логирующий декоратор
		http_handler::LoggingRequestHandler logging_handler{handler};

		// 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
		// http_server::ServeHttp(ioc, {address, port}, [&handler](auto&& req, const auto& socket, auto&& send) {
		// 	handler(std::forward<decltype(req)>(req), std::forward<decltype(socket)>(socket),
		// 		std::forward<decltype(send)>(send));
		// });

		http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, const auto& socket, auto&& send) {
			logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(socket)>(socket),
				std::forward<decltype(send)>(send));
		});

		// Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
		// std::cout << "Server has started..."sv << std::endl;
		srv_log::LogMessage({{"port", port}, {"address", address.to_string()}}, "server started"sv);

		// 6. Запускаем обработку асинхронных операций
		RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });

	} catch (const std::exception& ex) {

		std::cerr << ex.what() << std::endl;
		srv_log::LogMessage({{"code", EXIT_FAILURE}, {"exception", ex.what()}}, "server exited"sv);
		return EXIT_FAILURE;
	}

	srv_log::LogMessage({{"code", 0}}, "server exited"sv);
	
}
