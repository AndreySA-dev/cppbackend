#pragma once
#include "app_listener.h"
#include "game_handler.h"
#include "game_serialization.h"
#include <filesystem>

namespace fs = std::filesystem;
namespace net = boost::asio;

namespace saver {

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

};

} // namespace saver