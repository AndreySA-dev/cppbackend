#pragma once

#include <mutex>
#include <random>
#include <string>


namespace helper {

[[nodiscard]] std::string URLDecode(const std::string_view encoded);



template <typename T, bool is_thread_safe = true>
T GetRandomNum(T min, T max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	if constexpr (is_thread_safe) {
		static std::mutex mut;
		std::lock_guard<std::mutex> lock(mut);
	}
	std::uniform_real_distribution<> dist(min, max);

	return dist(gen);
}


bool LessOrEqual(double a, double b, double eps = 1e-9);
bool GreatOrEqual(double a, double b, double eps = 1e-9);

} // namespace helper