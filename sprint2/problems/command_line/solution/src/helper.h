#pragma once

#include <random>
#include <string>
#include <mutex>


namespace helper {

[[nodiscard]] std::string URLDecode(const std::string_view encoded);

// int random_int(int min, int max);
// int random_double(double min, double max);

template<typename T>
T GetRandomNum(T min, T max) {
	static std::mutex mut;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::lock_guard<std::mutex> lock(mut);
    std::uniform_real_distribution<> dist(min, max);

    return dist(gen);
}


bool LessOrEqual(double a, double b, double eps = 1e-9);
bool GreatOrEqual(double a, double b, double eps = 1e-9);

} // namespace helper