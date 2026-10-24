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
			tokens_to_users_id_.insert({new_token, new_player_ptr->GetId()});
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
	auto it = tokens_to_users_id_.find(token);
	return it != tokens_to_users_id_.cend() ? pair{users_.GetUser(it->second), Code::OK}
											: pair{nullptr, Code::PLAYER_NOT_FOUND};
}

std::pair<const user::User*, Code> Authenticator::GetUser(token::Token token) const {
	if (!TokenIsCorrect(token)) {
		return {nullptr, Code::TOKEN_IS_INCORRECT};
	}
	auto it = tokens_to_users_id_.find(token);
	return it != tokens_to_users_id_.cend() ? pair{users_.GetUser(it->second), Code::OK}
											: pair{nullptr, Code::PLAYER_NOT_FOUND};
}

user::Users& Authenticator::GetUsers() {
	return users_;
}

const user::Users& Authenticator::GetUsers() const {
	return users_;
}


bool Authenticator::TokenIsCorrect(const token::Token& token) const {
	return token::TokenIsCorrect(token);
}

model::Game Authenticator::GetGame() {
	return game_;
}

std::vector<token::Token> Authenticator::GetTokens() const {
	std::vector<token::Token> result;
	std::transform(tokens_to_users_id_.cbegin(), tokens_to_users_id_.cend(), std::back_inserter(result), [] (const auto& token) {
		return token.first;
	});
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
	tokens_to_users_id_.insert({token, user.GetId()});
	users_.AddUser(std::move(user));
}


} // namespace auth