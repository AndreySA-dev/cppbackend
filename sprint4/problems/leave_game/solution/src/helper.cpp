#include "helper.h"

#include <stdexcept>
#include <cmath>
#include <algorithm>

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

// generated  by https://www.perplexity.ai
std::unordered_map<std::string, std::string> ParseURLQuery(const std::string_view query) {
	std::unordered_map<std::string, std::string> params;

	size_t question_mark = query.find('?');
	if (query.empty() ||  question_mark == std::string::npos) {
		return params;
	}

	std::size_t start = ++question_mark;
	while (start < query.size()) {
		std::size_t ampersand = query.find('&', start);

		const std::size_t end = (ampersand == std::string::npos) ? query.size() : ampersand;

		const std::string_view pair = query.substr(start, end - start);

		if (!pair.empty()) {
			const std::size_t equals = pair.find('=');

			std::string_view key;
			std::string_view value;

			if (equals == std::string::npos) {
				key = pair;
			} else {
				key = pair.substr(0, equals);
				value = pair.substr(equals + 1);
			}

			params[URLDecode(key)] = URLDecode(value);
		}

		if (ampersand == std::string::npos) {
			break;
		}

		start = ampersand + 1;
	}

	return params;
}


bool LessOrEqual(double a, double b, double eps) {
	return (a < b) || (std::fabs(a - b) <= eps);
}

bool GreatOrEqual(double a, double b, double eps) {
	return LessOrEqual(b, a, eps);
}


} // namespace helper