#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <stdexcept>

using namespace std;
using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {

	auto GetTime() const {
		if (manual_ts_) {
			return *manual_ts_;
		}

		return std::chrono::system_clock::now();
	}

	auto GetTimeStamp() const {
		const auto now = GetTime();
		const auto t_c = std::chrono::system_clock::to_time_t(now);
		return std::put_time(std::localtime(&t_c), "%F %T");
	}

	// Для имени файла возьмите дату с форматом "%Y_%m_%d"
	std::string GetFileTimeStamp() const {

		std::time_t tt = std::chrono::system_clock::to_time_t(GetTime());
		std::tm tm = *std::localtime(&tt);
		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y_%m_%d");
		return oss.str();
	};

	Logger() = default;
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(Logger&&) = delete;


  public:
	static Logger& GetInstance() {
		static Logger obj;
		return obj;
	}

	// Выведите в поток все аргументы.
	template <class... Ts>
	void Log(const Ts&... args) {
		lock_guard lock(mutex_);
		OpenLogFile(GetTime());
		*file_ << GetTimeStamp() << ": ";
		(*file_ << ... << args);
		*file_ << endl;
	}

	// Установите manual_ts_. Учтите, что эта операция может выполняться
	// параллельно с выводом в поток, вам нужно предусмотреть
	// синхронизацию.
	void SetTimestamp(std::chrono::system_clock::time_point ts) {
		lock_guard lock(mutex_);
		manual_ts_ = ts;
	};

  private:
	const std::string file_name_prefix = "/var/log/sample_log_";
	// const std::string file_name_prefix = "";
	const std::string file_name_suffix = ".log";

	void OpenLogFile(chrono::system_clock::time_point tp) {

		std::string new_time_str = GetFileTimeStamp();

		if (!file_ || new_time_str != file_name_time_str) {
			std::string name = file_name_prefix + new_time_str + file_name_suffix;
			file_ = {ofstream(name, ios::app)};
			if (!*file_) {
				throw runtime_error("Error. Cant open log file");
			}
			file_name_time_str = std::move(new_time_str);

		}

	}

	std::optional<std::chrono::system_clock::time_point> manual_ts_;

	mutable std::mutex mutex_;

	std::string file_name_time_str = "";
	optional<ofstream> file_ = nullopt;

	chrono::system_clock::time_point next_day_border_time;
};
