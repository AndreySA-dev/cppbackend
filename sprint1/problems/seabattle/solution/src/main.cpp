#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
	auto left_pad = "  "s;
	auto delimeter = "    "s;
	std::cout << left_pad;
	SeabattleField::PrintDigitLine(std::cout);
	std::cout << delimeter;
	SeabattleField::PrintDigitLine(std::cout);
	std::cout << std::endl;
	for (size_t i = 0; i < SeabattleField::field_size; ++i) {
		std::cout << left_pad;
		left.PrintLine(std::cout, i);
		std::cout << delimeter;
		right.PrintLine(std::cout, i);
		std::cout << std::endl;
	}
	std::cout << left_pad;
	SeabattleField::PrintDigitLine(std::cout);
	std::cout << delimeter;
	SeabattleField::PrintDigitLine(std::cout);
	std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
	boost::array<char, sz> buf;
	boost::system::error_code ec;

	net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

	if (ec) {
		return std::nullopt;
	}

	return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
	boost::system::error_code ec;

	net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

	return !ec;
}

std::string ShotResultToStr(SeabattleField::ShotResult result) {
	std::string res_str;
	switch (result) {
	case SeabattleField::ShotResult::HIT:
		res_str = "HIT ";
		break;
	case SeabattleField::ShotResult::KILL:
		res_str = "KILL"sv;
		break;
	case SeabattleField::ShotResult::MISS:
		res_str = "MISS"sv;
		break;
	}
	return res_str;
}

std::ostream& operator<<(std::ostream& out, SeabattleField::ShotResult result) {
	return out << ShotResultToStr(result);
}

class SeabattleAgent {
  public:
	SeabattleAgent(const SeabattleField& field) : my_field_(field) {}

	void StartGame(tcp::socket& socket, bool my_initiative) {
		std::cout << "Game started\n"sv;
		PrintFieldPair(my_field_, other_field_);

		while (!IsGameEnded()) {
			if (my_initiative) {
				// my step opponent step

				std::string strike_coords_str;
				std::optional<std::pair<int, int>> coords;


				std::cout << "Enter coordinates for strike: "sv;
				for (;;) {
					std::cin >> strike_coords_str;
					if (coords = ParseMove(strike_coords_str)) {
						break;
					}
					std::cout << "Error: Incorrect coords '"sv << strike_coords_str << '\'' << std::endl;
				}

				std::cout << "Strike to "sv << strike_coords_str << std::endl;
				WriteExact(socket, strike_coords_str);

				auto response = ReadExact<1>(socket);

				if (!response) {
					std::cout << "Error: No response from opponent."sv << std::endl;
					socket.close();
					return;
				}

				auto [y, x] = *coords;

				if (response == "K") { // KILL

					other_field_.MarkKill(x, y);
					std::cout << "You kill ship"sv << std::endl;

				} else if (response == "H") {

					other_field_.MarkHit(x, y);
					std::cout << "You hit ship"sv << std::endl;

				} else if (response == "M") {

					other_field_.MarkMiss(x, y);
					std::cout << "You miss"sv << std::endl;
					my_initiative = false;
				}

				std::cout << '\n';
				PrintFieldPair(my_field_, other_field_);

			} else {
				// opponent step

				auto opponent_request = ReadExact<2>(socket);

				if (!opponent_request) {

					std::cout << "Error: No response from opponent."sv << std::endl;
					socket.close();
					return;
				}

				auto coords = ParseMove(*opponent_request);

				if (!coords) {

					std::cout << "Error: Opponent send incorrect coordinates: '"sv << *opponent_request << '\''
							  << std::endl;
					socket.close();
					return;
				}

				auto [y,x] = *coords;
				auto shot_result = my_field_.Shoot(x, y);
				std::string response;

				if (shot_result == SeabattleField::ShotResult::HIT) {
					response = "H";
				} else if (shot_result == SeabattleField::ShotResult::KILL) {
					response = "K";
				} else if (shot_result == SeabattleField::ShotResult::MISS) {
					response = "M";
				}

				WriteExact(socket, response);

				std::cout << "Opponent shot to "sv << MoveToString({y, x}) << " and "sv << ShotResultToStr(shot_result) << '\n';
				my_initiative = shot_result == SeabattleField::ShotResult::MISS;

				PrintFieldPair(my_field_, other_field_);
			}
		}

		std::cout << "Game over, ";
		if (other_field_.IsLoser()) {
			std::cout << "You WIN !!!\n"sv;
		} else {
			std::cout << "You Loose :(\n"sv;
		}
		socket.close();
	}

  private:
	static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
		if (sv.size() != 2)
			return std::nullopt;

		int p1 = sv[0] - 'A', p2 = sv[1] - '1';

		if (p1 < 0 || p1 > 8)
			return std::nullopt;
		if (p2 < 0 || p2 > 8)
			return std::nullopt;

		return {{p1, p2}};
	}

	static std::string MoveToString(std::pair<int, int> move) {
		char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
		return {buff, 2};
	}

	void PrintFields() const {
		PrintFieldPair(my_field_, other_field_);
	}

	bool IsGameEnded() const {
		return my_field_.IsLoser() || other_field_.IsLoser();
	}

	// TODO: добавьте методы по вашему желанию

  private:
	SeabattleField my_field_;
	SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
	SeabattleAgent agent(field);

	net::io_context io_context;

	tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
	std::cout << "Waiting for opponent..."sv << std::endl;

	boost::system::error_code ec;
	tcp::socket socket{io_context};
	acceptor.accept(socket, ec);

	if (ec) {
		std::cout << "Can't accept opponent connection"sv << std::endl;
		exit(1);
	}
	std::cout << "Opponent connected..."sv << std::endl;

	agent.StartGame(socket, false);
};

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
	SeabattleAgent agent(field);

	boost::system::error_code ec;
	auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);

	if (ec) {
		std::cout << "Wrong IP format"sv << std::endl;
		exit(1);
	}

	net::io_context io_context;
	tcp::socket socket{io_context};
	socket.connect(endpoint, ec);

	if (ec) {
		std::cout << "Can't connect to server"sv << std::endl;
		exit(1);
	}

	agent.StartGame(socket, true);

	// socket.write_some(net::buffer("Hello, I'm client!\n"sv), ec);
	// if (ec) {
	// 	std::cout << "Error sending data"sv << std::endl;
	// 	return 1;
	// }

	// net::streambuf stream_buf;
	// net::read_until(socket, stream_buf, '\n', ec);
	// std::string server_data{std::istreambuf_iterator<char>(&stream_buf),
	// 						std::istreambuf_iterator<char>()};

	// if (ec) {
	// 	std::cout << "Error reading data"sv << std::endl;
	// 	return 1;
	// }

	// std::cout << "Server responded: "sv << server_data << std::endl;

	// agent.StartGame(socket, true);
};

int main(int argc, const char** argv) {
	using namespace std;
	if (argc != 3 && argc != 4) {
		std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
		return 1;
	}

	std::mt19937 engine(std::stoi(argv[1]));
	SeabattleField fieldL = SeabattleField::GetRandomField(engine);

	if (argc == 3) {
		StartServer(fieldL, std::stoi(argv[2]));
	} else if (argc == 4) {
		StartClient(fieldL, argv[2], std::stoi(argv[3]));
	}

	// SeabattleField fieldR;

	// PrintFieldPair(fieldL, fieldR);

	// cout << "1 ---\n" << endl;
	// auto shoot_result = fieldL.Shoot(2, 0);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(7, 1);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(7, 3);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(1, 5);
	// std::cout << shoot_result << std::endl;
	// PrintFieldPair(fieldL, fieldR);

	// cout << "2 ---\n" << endl;
	// shoot_result = fieldL.Shoot(2, 2);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(3, 2);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(3, 5);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(3, 6);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(0, 7);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(1, 7);
	// std::cout << shoot_result << std::endl;
	// PrintFieldPair(fieldL, fieldR);

	// cout << "3 ---\n" << endl;
	// shoot_result = fieldL.Shoot(5, 0);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(5, 1);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(5, 2);

	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(0, 1);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(0, 2);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(0, 3);
	// std::cout << shoot_result << std::endl;
	// PrintFieldPair(fieldL, fieldR);

	// cout << fieldL.IsLoser() << endl;

	// cout << "4 ---\n" << endl;
	// shoot_result = fieldL.Shoot(5, 4);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(5, 5);
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(5, 6);
	// cout << fieldL.IsLoser() << endl;
	// std::cout << shoot_result << std::endl;
	// shoot_result = fieldL.Shoot(5, 7);
	// PrintFieldPair(fieldL, fieldR);

	// cout << fieldL.IsLoser() << endl;

	// fieldR.MarkHit(0, 0);
	// fieldR.MarkHit(0, 1);
	// fieldR.MarkHit(0, 2);
	// fieldR.MarkHit(0, 3);
	// fieldR.MarkHit(0, 4);
	// fieldR.MarkHit(0, 5);
	// fieldR.MarkHit(0, 6);
	// fieldR.MarkHit(0, 7);
	// PrintFieldPair(fieldL, fieldR);
	// cout << fieldR.IsLoser() << endl;

	// fieldR.MarkHit(1, 0);
	// fieldR.MarkHit(1, 1);
	// fieldR.MarkHit(1, 2);
	// fieldR.MarkHit(1, 3);
	// fieldR.MarkHit(1, 4);
	// fieldR.MarkHit(1, 5);
	// fieldR.MarkHit(1, 6);
	// fieldR.MarkHit(1, 7);
	// PrintFieldPair(fieldL, fieldR);
	// cout << fieldR.IsLoser() << endl;

	// fieldR.MarkHit(2, 0);
	// fieldR.MarkHit(2, 1);
	// fieldR.MarkHit(2, 2);
	// cout << fieldR.IsLoser() << endl;
	// fieldR.MarkHit(2, 3);
	// cout << fieldR.IsLoser() << endl;

	// PrintFieldPair(fieldL, fieldR);

	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
	// fieldR.MarkHit(0,0);
}
