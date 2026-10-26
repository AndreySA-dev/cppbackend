#include "run.h"
#include <boost/json.hpp>
#include <iostream>
#include <optional>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

using namespace std;
using namespace std::literals;
using pqxx::operator"" _zv;

namespace json = boost::json;

enum class Action { ADD_BOOK, ALL_BOOK, EXIT, UNKNOWN };

constexpr auto tag_ins_book = "ins_book"_zv;
constexpr auto ok_result_str = "{\"result\":true}";
constexpr auto fail_result_str = "{\"result\":false}";

const std::unordered_map<std::string_view, Action> STR_TO_ACTIONS = {
	{"add_book"sv, Action::ADD_BOOK},
	{"all_books"sv, Action::ALL_BOOK},
	{"exit"sv, Action::EXIT},
};

using ParseResult = std::pair<Action, json::object>;

ParseResult ParseCommand(const string& line) {

	try {
		json::value line_jo = json::parse(line).as_object();
		auto action = line_jo.at("action").as_string();
		auto act_it = STR_TO_ACTIONS.find(action);
		if (act_it != STR_TO_ACTIONS.cend()) {
			auto payload_jo = line_jo.at("payload").as_object();
			return {act_it->second, payload_jo};
		}
	} catch (const exception& e) { throw std::invalid_argument("Error. Can't parse json command"s); }

	return {Action::UNKNOWN, {}};
}

std::pair<std::string, bool> DBRequest(pqxx::connection& conn, const ParseResult& req) {
	auto& action = req.first;
	auto& payload = req.second;

	try {
		if (action == Action::ADD_BOOK) {

			pqxx::work work(conn);
			string title = std::string(payload.at("title").as_string());
			string author = std::string(payload.at("author").as_string());
			int year = payload.at("year").to_number<int>();
			auto isbn_jv = payload.at("ISBN");
			optional<string> isbn = nullopt;
			if (isbn_jv.is_string()) {
				isbn = isbn_jv.as_string();
			}
			work.exec_prepared(tag_ins_book, title, author, year, isbn);
			work.commit();
			return {ok_result_str, true};

		} else if (action == Action::ALL_BOOK) {

			static const auto query_text =
				"SELECT id, title, author, year, ISBN FROM books ORDER by year DESC, title, author, ISBN"_zv;
			pqxx::read_transaction read{conn};
			json::array resp_ja;
			for (auto [id, title, author, year, isbn] :
				read.query<int, string, string, int, optional<string>>(query_text)) {
				json::object book_jo;
				book_jo["id"] = id;
				book_jo["title"] = title;
				book_jo["author"] = author;
				book_jo["year"] = year;
				if (isbn) {
					book_jo["ISBN"] = *isbn;
				} else {
					book_jo["ISBN"] = nullptr;
				}
				resp_ja.emplace_back(book_jo);
			}
			return {json::serialize(resp_ja), true};
		}
	} catch (const std::exception& e) {	 //
										 // cerr << "Parsing error: " << e.what() << endl;
	} catch (const pqxx::sql_error& e) { //
										 // cerr << "SQL error: " << e.what() << endl;
	}

	return {fail_result_str, false};
}

void Run(std::string conn_str) {


	pqxx::connection conn{conn_str};
	pqxx::work w{conn};
	w.exec(R"(
		CREATE TABLE IF NOT EXISTS books (
			id SERIAL PRIMARY KEY,
			title varchar(40) NOT NULL,
			author varchar(100) NOT NULL,
			year integer NOT NULL,
			ISBN char(13) UNIQUE
		);)"_zv);
	w.commit();
	conn.prepare(tag_ins_book, "INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, $4)"_zv);

	string line;
	while (getline(cin, line)) {
		ParseResult req{Action::UNKNOWN, {}};
		try {
			req = ParseCommand(line);
		} catch (const std::exception) {
			cout << fail_result_str << endl;
			continue;
		}

		if (req.first == Action::UNKNOWN) {
			break;
		} else if (req.first == Action::EXIT) {
			break;
		} else {
			auto resp = DBRequest(conn, req);
			cout << resp.first << endl;
		}
	}
}
