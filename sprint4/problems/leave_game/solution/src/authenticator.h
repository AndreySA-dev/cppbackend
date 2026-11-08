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

	Authenticator(model::Game& game);
	std::pair<token::Token, Code> AddUser(std::string_view name, model::Map::Id map_id);
	std::pair<user::User*, Code> GetUser(token::Token token);
	std::pair<const user::User*, Code> GetUser(token::Token token) const;
	const user::Users& GetUsers() const;
	const user::User* GetUser(user::User::Id id) const;
	bool TokenIsCorrect(const token::Token& token) const;
	model::Game GetGame();
	std::vector<token::Token> GetTokens() const; // TO-DO make a iterator 
	void RestorePlayer(user::User user, model::Map::Id map_id, token::Token token);
	void RetirePlayer(user::User::Id user_id);

  private:
	using TokensToUserIdIdx = std::unordered_map<token::Token, user::User::Id, token::TokenHasher>;
	using UserIdToTokenIdx = std::unordered_map<user::User::Id, token::Token, user::User::IdHasher>;

	TokensToUserIdIdx token_to_users_id_idx_;
	UserIdToTokenIdx user_id_to_token_idx_;
	user::Users users_;
	token::TokenHandler generator_;
	model::Game& game_;
};


} // namespace auth