#include "token.h"

Token TokenGenerator::GetNewToken() {

	uint64_t part1 = generator1_();
	uint64_t part2 = generator2_();

	std::string token_str(sizeof(uint64_t) * 8 / 4 * 2, ' ');
	// sizeof(uint64_t) * 8 / 4 * 2 = 8 byte * 8bit / 4bit_in_HEX * 2times = 32 symbol

	NumToHexString(part1, token_str, 0);
	NumToHexString(part2, token_str, 16);
	return Token{std::move(token_str)};
}
