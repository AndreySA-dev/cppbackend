#pragma once


#include <cassert>
#include <random>
#include <string>

#include "tagged.h"


namespace token {

namespace detail {

using namespace std::literals;

struct TokenTag {};
const size_t TOCKEN_SIZE = 32;
const std::string HEX_CHARS = "0123456789abcdef"s;

} // namespace detail

using namespace std::literals;

using Token = util::Tagged<std::string, detail::TokenTag>;
using TokenHasher = util::TaggedHasher<Token>;

bool TokenIsCorrect(const Token& token);
bool TokenIsCorrect(const std::string& token);

class TokenHandler {
  public:
	TokenHandler() = default;
	// TokenGenerator(const TokenGenerator&) = delete;
	// TokenGenerator(TokenGenerator&&) = delete;
	// TokenGenerator& operator=(const TokenGenerator&) = delete;
	// TokenGenerator& operator=(TokenGenerator&&) = delete;

	Token GetNewToken();


  private:
	template <typename T>
	void NumToHexString(T number, std::string& str, size_t pos) {

		constexpr int char_num = sizeof(T) * 8 / 4; // 4 bit in HEX char

		assert(pos + char_num <= str.size());

		size_t hi_bound = pos + char_num;

		for (; pos < hi_bound; ++pos) {
			str[pos] = detail::HEX_CHARS[number & 0xF]; // get only 4 lower bits (from numeric 0 to 15)
			number = number >> 4;
		}
	}

	std::random_device random_device_;
	std::mt19937_64 generator1_{[this] {
		std::uniform_int_distribution<std::mt19937_64::result_type> dist;
		return dist(random_device_);
	}()};

	std::mt19937_64 generator2_{[this] {
		std::uniform_int_distribution<std::mt19937_64::result_type> dist;
		return dist(random_device_);
	}()};
};


} // namespace token