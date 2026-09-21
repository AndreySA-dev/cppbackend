#pragma once

#include "model.h"
#include "player.h"
#include "codes.h"

#include <deque>
#include <unordered_map>
#include <string_view>
#include <utility>
#include "token.h"

namespace auth {

class Authenticator {
	public:

	// enum class Code {
	// 	OK,
	// 	TOKEN_IS_INCORRECT,
	// 	PLAYER_NOT_FOUND
	// };

	Authenticator(model::Game& game);
	std::pair<token::Token, Code> AddPlayer(std::string_view name, model::Map::Id map_id);
	std::pair<player::Player*, Code> GetPlayer(token::Token token);
	player::Players& GetPlayers();
	bool TokenIsCorrect(const token::Token& token);


  private:
	std::unordered_map<token::Token, player::Player::Id, token::TokenHasher> tokens_to_player_id_;
	player::Players players_;
	token::TokenHandler generator_;
	model::Game& game_;
};


} // namespace auth