#include "game_saver.h"
#include "game_serialization.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#ifdef MYDEBUG
#include <iostream>
#endif

namespace saver {

namespace sr = serialization;
namespace fs = std::filesystem;
using InputArchive = boost::archive::binary_iarchive;
using OutputArchive = boost::archive::binary_oarchive;

FileSaver::FileSaver(game_handler::GameHandler& game_h, fs::path save_path, net::io_context& ioc)		 //
	: game_h_(game_h), save_path_(std::move(save_path)), ioc_(ioc), save_strand_(net::make_strand(ioc_)) //
{}

void FileSaver::SaveState() {

	auto game_repr = std::make_shared<sr::GameRepr>(game_h_.GetGame());
	auto auth_repr = std::make_shared<sr::AuthRepr>(game_h_.GetAuthenticator());

	auto file_save = [game_repr, auth_repr, save_path = save_path_]() {
		fs::path tmp_file_path = save_path.parent_path() / ("~" + save_path.filename().string());
		bool is_save_correct = false;
		if (auto ofile = std::ofstream(tmp_file_path.string(), std::ios::binary)) {
			OutputArchive oa{ofile};
			oa << *game_repr;
			oa << *auth_repr;
			is_save_correct = true;
		}

		if (is_save_correct) {
			fs::rename(tmp_file_path, save_path);
		}
	};

	if (!ioc_.stopped()) {

#ifdef MYDEBUG
		auto now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(now);
		std::cerr << std::ctime(&t) << ". Async state save" << std::endl;
#endif
		net::post(save_strand_, file_save);

	} else {

#ifdef MYDEBUG
		auto now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(now);
		std::cerr << std::ctime(&t) << ". Sync state save" << std::endl;
#endif
		file_save();
	}
}

void FileSaver::LoadState() {

	if (!fs::exists(save_path_)) {
		throw std::invalid_argument("Load state error: file " + save_path_.string() + " not found.");
	}

	sr::GameRepr game_repr;
	sr::AuthRepr auth_repr;

	if (auto ifile = std::ifstream(save_path_.string(), std::ios::binary)) {
		InputArchive ia{ifile};
		ia >> game_repr;
		ia >> auth_repr;
	}
	game_repr.Restore(game_h_.GetGame()); // game need to be loaded first
	auth_repr.Restore(game_h_.GetAuthenticator());
}


FileSerializationListener::FileSerializationListener(
	game_handler::GameHandler& game_h, fs::path save_path, net::io_context& ioc, std::chrono::milliseconds save_period)
	: file_saver_(game_h, std::move(save_path), ioc), save_period_(save_period) {
	//
}

void FileSerializationListener::OnTick(std::chrono::milliseconds duration) {
	past_from_save_ += duration;
	if (past_from_save_ >= save_period_) {
		file_saver_.SaveState();
#ifdef MYDEBUG
		std::cerr << "OnTick Save" << std::endl;
#endif
		past_from_save_ = std::chrono::milliseconds{0};
	}
}

} // namespace saver