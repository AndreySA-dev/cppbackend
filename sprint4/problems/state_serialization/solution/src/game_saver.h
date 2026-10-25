#pragma once
#include "app_listener.h"
#include "game_handler.h"
#include "game_serialization.h"
#include <filesystem>

namespace fs = std::filesystem;
namespace net = boost::asio;

namespace saver {

// void SaveState(std::shared_ptr<game_handler::GameHandler> game_h, fs::path file_path, net::io_context& ioc /*,
// 	net::strand<net::io_context::executor_type>& game_strand */);

// void LoadState(std::shared_ptr<game_handler::GameHandler> game_h, fs::path file_path, net::io_context& ioc /*,
// 	net::strand<net::io_context::executor_type>& game_strand */);

// class IStateSaver {

//   public:
// 	virtual void SaveState() = 0;
// 	virtual void LoadState() = 0;

//   private:
// 	game_handler::GameHandler& game_h_;
// };

class FileSaver {
  public:
	FileSaver(game_handler::GameHandler& game_h, fs::path save_path, net::io_context& ioc);
	FileSaver(const FileSaver&) = delete;
	FileSaver(FileSaver&&) = delete;
	FileSaver operator=(const FileSaver&) = delete;
	FileSaver operator=(FileSaver&&) = delete;

	void SaveState();
	void LoadState();

  private:
	game_handler::GameHandler& game_h_;
	fs::path save_path_;
	net::io_context& ioc_;
	net::strand<net::io_context::executor_type> save_strand_;
};


class FileSerializationListener : public listener::IApplicationListener {

  public:
	FileSerializationListener(game_handler::GameHandler& game_h, fs::path save_path, net::io_context& ioc, std::chrono::milliseconds save_period);
	void OnTick(std::chrono::milliseconds duration) override;

  private:
	FileSaver file_saver_;
	std::chrono::milliseconds save_period_;
	std::chrono::milliseconds past_from_save_;
	// std::chrono::steady_clock::time_point prev_save_time_;

};

} // namespace saver