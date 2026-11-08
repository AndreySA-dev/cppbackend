#include "retired_db.h"
#include <boost/asio.hpp>
#include <chrono>
#include <pqxx/pqxx>
#include <pqxx/zview.hxx>
#include <stdexcept>
#include <string_view>

namespace db {

using namespace std;
using namespace std::literals;
using pqxx::operator"" _zv;


DB::DB(const std::string& connection_url, net::io_context& ioc, Private)
	: read_connection_(connection_url), write_connection_(connection_url), strand_(net::make_strand(ioc)) {
	pqxx::work work{write_connection_};
	work.exec("CREATE TABLE IF NOT EXISTS retired ("
			  "name varchar(64) NOT NULL,"
			  "score integer NOT NULL,"
			  "play_time float4 NOT NULL"
			  ");"_zv);

	work.commit();
}

DB::Ptr DB::Create(const std::string& connection_url, net::io_context& ioc) {
	return std::make_shared<DB>(connection_url, ioc, Private());
}

DB::Transaction DB::GetReadTransaction() {
	return GetTransaction(OperationType::READ);
}

DB::Transaction DB::GetWriteTransaction() {
	return GetTransaction(OperationType::WRITE);
}

// std::shared_ptr<DB::Transaction> DB::GetTransactionPtr() {
// 	if (write_locked_) {
// 		throw std::logic_error("Error. Write connection is busy.");
// 	}
// 	write_locked_ = true;
// 	return std::make_shared<DB::Transaction>(shared_from_this(), write_connection_, OperationType::WRITE);
// }

net::strand<net::io_context::executor_type>& DB::GetStrand() {
	return strand_;
}

DB::Transaction DB::GetTransaction(OperationType type) {
	if (type == OperationType::READ) {
		if (read_locked_) {
			throw std::logic_error("Error. Read connection is busy.");
		}
		read_locked_ = true;
		return DB::Transaction(shared_from_this(), read_connection_, OperationType::READ);
	} else {
		if (write_locked_) {
			throw std::logic_error("Error. Write connection is busy.");
		}
		write_locked_ = true;
		return DB::Transaction(shared_from_this(), write_connection_, OperationType::WRITE);
	}
}

DB::Transaction::Transaction(DB::Ptr db, pqxx::connection& connection, OperationType op_type)
	: db_ptr_(db), work_(connection), op_type_(op_type) {}

DB::OperationType DB::Transaction::GetOpType() const {
	return op_type_;
}

bool DB::Transaction::SaveRetired(const gh::GameHandler::RetiredPlayers&& retired) {
	if (is_utilized_) {
		throw std::logic_error("Error. Unit of work can't be used two times.");
	}
	for (const auto& player : retired) {
		work_.exec_params("INSERT INTO retired (name, score, play_time) VALUES ($1, $2, $3);"_zv, player.name,
			player.score, player.play_time);
	}
	work_.commit();
	is_utilized_ = true;
	return false;
}

gh::GameHandler::RetiredPlayers DB::Transaction::LoadRetired(size_t start_record, size_t max_records) {
	if (is_utilized_) {
		throw std::logic_error("Error. Unit of work can't be used two times.");
	}
	std::vector<gh::RetiredPlayer> result;

	string query;
	query = "SELECT name, score, play_time FROM retired"
			" ORDER BY score DESC, play_time ASC, name ASC"
			" OFFSET " +
			std::to_string(start_record);
	if (max_records > 0) {
		query += " LIMIT " + std::to_string(max_records);
	}
	query += ";";

	for (auto [name, score, time] : work_.query<string_view, size_t, double>(query)) {
		result.emplace_back(std::string{name}, score, time);
	}
	work_.commit();
	is_utilized_ = true;
	return result;
}

// void DB::UnitOfWork::Commit() {
// 	work_.commit();
// }

DB::Transaction::~Transaction() {
	// unblock connection in DB
	if (op_type_ == DB::OperationType::READ) {
		db_ptr_->read_locked_ = false;
	} else {
		db_ptr_->write_locked_ = false;
	}
}

DBRetiredPlayerSaverListener::DBRetiredPlayerSaverListener(DB::Ptr db, gh::GameHandler& game_hndl, net::io_context& ioc)
	: db_(db), game_hndl_(game_hndl), ioc_(ioc), write_strand_(net::make_strand(ioc_)) {}

void DBRetiredPlayerSaverListener::OnTick([[maybe_unused]] std::chrono::milliseconds duration) {

	if (game_hndl_.HasRetired()) {
		// auto p = db_->GetWriteUnitOfWorkPtr();
		// auto write_functor = [write_unit = std::move(p), players = std::move(game_hndl_.PopRetired())]() {
		// 	write_unit->SaveRetired(std::move(players));
		// };
		auto transaction = db_->GetWriteTransaction();
		transaction.SaveRetired(game_hndl_.PopRetired());


		// 		if (!ioc_.stopped()) {
		// #ifdef MYDEBUG
		// 			// auto now = std::chrono::system_clock::now();
		// 			// std::time_t t = std::chrono::system_clock::to_time_t(now);
		// 			// std::cerr << std::ctime(&t) << ". Async save retired players" << std::endl;
		// #endif
		// 			net::post(write_strand_, std::move(write_functor));
		// 		}
	}
}

json::array RetiredPlayersToJSONA(const gh::GameHandler::RetiredPlayers& players) {
	json::array result;

	for (const auto& player : players) {
		result.emplace_back(
			json::object{{"name", player.name}, {"score", player.score}, {"playTime", player.play_time}});
	}
	return result;
	// return json::array{};
}

} // namespace db
