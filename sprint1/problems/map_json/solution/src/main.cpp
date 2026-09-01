#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "json_loader.h"
#include "request_handler.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

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

// StringResponse HandleRequest(StringRequest&& req) {
// 	const auto text_response = [&req](http::status status, std::string_view text) {
// 		return MakeStringResponse(status, text, req.version(), req.keep_alive());
// 	};

// 	// std::cerr << "HandleRequest" << std::endl;

// 	StringResponse resp;

// 	if (req.method() == http::verb::get || req.method() == http::verb::head) {
// 		std::stringstream ss;
// 		ss << "Hello, "sv << req.target().substr(1);

// 		resp = text_response(http::status::ok, ss.str());
// 		if (req.method() == http::verb::head) {
// 			resp.body() = "";
// 		}
// 	} else {
// 		resp = text_response(http::status::method_not_allowed, "Invalid method");
// 		resp.set(http::field::allow, "GET,HEAD");
// 	}
// 	return resp;
// }

} // namespace

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: game_server <game-config-json>"sv << std::endl;
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

		// 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
		http_handler::RequestHandler handler{game};

		// 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
		http_server::ServeHttp(ioc, {address, port}, [&handler](auto&& req, auto&& send) {
			handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
		});

		// Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
		std::cout << "Server has started..."sv << std::endl;

		// 6. Запускаем обработку асинхронных операций
		RunWorkers(std::max(1u, num_threads), [&ioc] { ioc.run(); });
	} catch (const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
}
