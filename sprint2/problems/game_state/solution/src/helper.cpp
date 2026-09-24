#include "helper.h"
#include <stdexcept>
#include <mutex>

// Helper function to convert a hexadecimal character to an integer
namespace helper {

namespace {

int HexValue(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	throw std::invalid_argument("Invalid hexadecimal value");
}

} // namespace

std::string URLDecode(const std::string_view encoded) {
	// Create a buffer to hold the decoded string
	std::string decoded;
	decoded.reserve(encoded.size());

	// Decode the URL-encoded string
	for (auto it = encoded.begin(); it != encoded.end(); ++it) {
		if (*it == '%') {
			// Check if we have at least two more characters after %
			if ((std::distance(it, encoded.end()) >= 3) && std::isxdigit(*(it + 1)) && std::isxdigit(*(it + 2))) {
				// Convert the two hexadecimal digits to an integer
				char byte = (HexValue(*(it + 1)) << 4) | HexValue(*(it + 2));
				decoded += byte;
				it += 2; // Skip the two hexadecimal digits we just processed
			} else {
				// If not valid, copy '%' as is
				decoded += *it;
			}
		} else if (*it == '+') {
			// Replace '+' with space
			decoded += ' ';
		} else {
			// Copy other characters as is
			decoded += *it;
		}
	}

	return decoded;
}

int random_int(int min, int max) {
	static std::mutex mut;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::lock_guard<std::mutex> lock(mut);
    std::uniform_real_distribution<> dist(min, max);

    return dist(gen);
}

} // namespace helper