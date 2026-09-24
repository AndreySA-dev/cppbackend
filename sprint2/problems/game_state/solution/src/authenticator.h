#pragma once

#include "model.h"
#include "user.h"
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
	std::pair<token::Token, Code> AddUser(std::string_view name, model::Map::Id map_id);
	std::pair<user::User*, Code> GetUser(token::Token token);
	user::Users& GetUsers();
	bool TokenIsCorrect(const token::Token& token);


  private:
	std::unordered_map<token::Token, user::User::Id, token::TokenHasher> tokens_to_users_id_;
	user::Users users_;
	token::TokenHandler generator_;
	model::Game& game_;
};


} // namespace auth