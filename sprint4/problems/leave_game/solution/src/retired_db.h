#pragma once
#include "app_listener.h"
#include "game_handler.h"
#include <boost/json.hpp>
#include <memory>
#include <pqxx/connection>
#include <pqxx/transaction>
#include <string>
#include <vector>

namespace db {

namespace gh = game_handler;
namespace net = boost::asio;
namespace json = boost::json;

class DB : public std::enable_shared_from_this<DB> {
	struct Private {
		explicit Private() = default;
	};
	class Transaction; // predeclare

  public:
	using Ptr = std::shared_ptr<DB>;
	
	// Constructor is only usable by this class (https://cppreference.com/)
	DB(const std::string& connection_url, net::io_context& ioc, Private);

	static Ptr Create(const std::string& connection_url, net::io_context& ioc);

	Transaction GetReadTransaction();
	Transaction GetWriteTransaction();
	// std::shared_ptr<Transaction> GetTransactionPtr();
	net::strand<net::io_context::executor_type>& GetStrand();

  private:
	enum class OperationType { READ, WRITE };

	Transaction GetTransaction(OperationType type);

	class Transaction {
	  public:
		Transaction(DB::Ptr db_ptr, pqxx::connection& connection, OperationType op_type);
		OperationType GetOpType() const;

		bool SaveRetired(const gh::GameHandler::RetiredPlayers&& retired);
		gh::GameHandler::RetiredPlayers LoadRetired(size_t start_record = 1, size_t max_records = 0);

		~Transaction();

		Transaction(const Transaction&) = delete;
		Transaction(Transaction&&) = delete;
		Transaction& operator=(const Transaction&) = delete;
		Transaction& operator=(Transaction&&) = delete;

	  private:
		DB::Ptr db_ptr_;
		pqxx::work work_;
		OperationType op_type_;
		bool is_utilized_ = false;
	};


	pqxx::connection read_connection_;
	pqxx::connection write_connection_;
	bool read_locked_ = false;
	bool write_locked_ = false;
	net::strand<net::io_context::executor_type> strand_;
};


class DBRetiredPlayerSaverListener : public listener::IApplicationListener {

  public:
	DBRetiredPlayerSaverListener(DB::Ptr db, gh::GameHandler& game_hndl, net::io_context& ioc);
	void OnTick([[maybe_unused]] std::chrono::milliseconds duration) override;

  private:
	DB::Ptr db_;
	gh::GameHandler& game_hndl_;
	net::io_context& ioc_;
	net::strand<net::io_context::executor_type> write_strand_;
};


json::array RetiredPlayersToJSONA(const gh::GameHandler::RetiredPlayers& palyers);


} // namespace db
