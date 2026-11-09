#include "authenticator.h"

#include <utility>


namespace auth {

using namespace std;


Authenticator::Authenticator(model::Game& game) : game_{game} {}


pair<token::Token, Code> auth::Authenticator::AddUser(string_view name, model::Map::Id map_id) {

	auto map_ptr = game_.FindMap(map_id);

	if (map_ptr) {
		if (auto new_player_ptr = users_.AddUser(name)) {
			auto new_token = generator_.GetNewToken();
			token_to_users_id_idx_.insert({new_token, new_player_ptr->GetId()});
			user_id_to_token_idx_.insert({new_player_ptr->GetId(), new_token});
			new_player_ptr->SetMap(map_ptr);
			return {new_token, auth::Code::OK};
		}
	}

	return {token::Token(""), Code::MAP_NOT_FOUND};
}


std::pair<user::User*, Code> Authenticator::GetUser(token::Token token) {
	if (!TokenIsCorrect(token)) {
		return {nullptr, Code::TOKEN_IS_INCORRECT};
	}
	auto it = token_to_users_id_idx_.find(token);
	return it != token_to_users_id_idx_.cend() ? pair{users_.GetUser(it->second), Code::OK}
											   : pair{nullptr, Code::PLAYER_NOT_FOUND};
}

std::pair<const user::User*, Code> Authenticator::GetUser(token::Token token) const {
	if (!TokenIsCorrect(token)) {
		return {nullptr, Code::TOKEN_IS_INCORRECT};
	}
	auto it = token_to_users_id_idx_.find(token);
	return it != token_to_users_id_idx_.cend() ? pair{users_.GetUser(it->second), Code::OK}
											   : pair{nullptr, Code::PLAYER_NOT_FOUND};
}

const user::Users& Authenticator::GetUsers() const {
	return users_;
}

const user::User* Authenticator::GetUser(user::User::Id id) const {
	return users_.GetUser(id);
}


bool Authenticator::TokenIsCorrect(const token::Token& token) const {
	return token::TokenIsCorrect(token);
}

model::Game Authenticator::GetGame() {
	return game_;
}

std::vector<token::Token> Authenticator::GetTokens() const {
	std::vector<token::Token> result;
	std::transform(token_to_users_id_idx_.cbegin(), token_to_users_id_idx_.cend(), std::back_inserter(result),
		[](const auto& token) { return token.first; });
	return result;
}

void Authenticator::RestorePlayer(user::User user, model::Map::Id map_id, token::Token token) {
	auto map_ptr = game_.FindMap(map_id);
	if (!map_ptr) {
		throw std::out_of_range("Restore player error. No map with name '" + *map_id + "'");
	}

	auto dog_ptr = map_ptr->GetDog(model::Dog::Id(*user.GetId()));
	if (!dog_ptr) {
		throw std::out_of_range("Restore player error. No dog on map with id '" + std::to_string(*user.GetId()) + "'");
	}

	user.SetMap(map_ptr);
	token_to_users_id_idx_.insert({token, user.GetId()});
	users_.AddUser(std::move(user));
	user.SetNextId(users_.GetSize());
}

void Authenticator::RetirePlayer(user::User::Id user_id) {
	users_.RetireUser(user_id);
	auto id_to_token_it = user_id_to_token_idx_.find(user_id);
	auto user_token = id_to_token_it->second;
	auto token_to_id_it = token_to_users_id_idx_.find(user_token);

	user_id_to_token_idx_.erase(id_to_token_it);
	token_to_users_id_idx_.erase(token_to_id_it);
}


} // namespace auth