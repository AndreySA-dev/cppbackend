#include "htmldecode.h"

#include <string_view>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace std::literals;

namespace {

// clang-format off
vector<pair<string, char>> mnemonics = {
	{"amp"s, '&'},
	{"AMP"s, '&'},
	{"lt"s, '<'},
	{"LT"s, '<'},
	{"gt"s, '>'},
	{"GT"s, '>'},
	{"apos"s, '\''},
	{"APOS"s, '\''},
	{"quot"s, '\"'},
	{"QUOT"s, '\"'}};
// clang-format on

} // namespace


std::string HtmlDecode(std::string_view str) {
	string result;
	result.reserve(str.length());

	for (auto it = str.cbegin(); it < str.cend();) {
		if (*it != '&') {
			result.push_back(*it);
			++it;
		} else {
			bool mnemonic_case = false;
			string_view tail = {it + 1, str.end()};
			for (const auto& mnemonic : mnemonics) {
				if (tail.starts_with(mnemonic.first)) {
					mnemonic_case = true;
					result.push_back(mnemonic.second);
					std::advance(it, mnemonic.first.length() + 1);
					if (it != str.end() && *it == ';') {
						++it;
					}
					continue;
				}
			}
			if (!mnemonic_case) {
				result.push_back('&');
				++it;
			}
		}
	}

	return result;
}
