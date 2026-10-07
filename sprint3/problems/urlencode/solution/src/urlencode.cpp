#include "urlencode.h"

#include <sstream>
// #include <iomanip>
// #include <cctype>

std::string UrlEncode(std::string_view str) {
	static const char* hex = "0123456789ABCDEF";

	std::string result;
	result.reserve(str.size() * 3); // worst case: all encoded

	for (unsigned char c : str) {
		// Unreserved characters (RFC 3986) are left as-is
		if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
			result.push_back(static_cast<char>(c));
		} else {
			// Percent-encode everything else
			result.push_back('%');
			result.push_back(hex[c >> 4]);
			result.push_back(hex[c & 0x0F]);
		}
	}

	return result;
}