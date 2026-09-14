#pragma once


#include <random>
#include <string>
#include <cassert>

#include "tagged.h"


namespace detail {

struct TokenTag {};

}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class TokenGenerator {
  public:
	TokenGenerator() = default;
	TokenGenerator(const TokenGenerator &) = delete;
	TokenGenerator(TokenGenerator &&) = delete;
	TokenGenerator &operator=(const TokenGenerator &) = delete;
	TokenGenerator &operator=(TokenGenerator &&) = delete;

	Token GetNewToken();

  private:

	template <typename T>
	void NumToHexString(T number, std::string &str, size_t pos) {

		const char hex_chars[] = "0123456789abcdef";
		constexpr int char_num = sizeof(T) * 8 / 4; // 4 bit in HEX char

		assert(pos + char_num <= str.size());

		size_t hi_bound = pos + char_num;

		for (; pos < hi_bound; ++pos) {
			str[pos] = hex_chars[number & 0xF]; // get only 4 lower bits (from numeric 0 to 15)
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