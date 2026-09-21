#include "authenticator.h"

#include <utility>


namespace auth {

using namespace std;


Authenticator::Authenticator(model::Game& game) : game_{game} {}


pair<token::Token, Code> auth::Authenticator::AddPlayer(string_view name, model::Map::Id map_id) {

	auto map_ptr = game_.FindMap(map_id);

	if (map_ptr) {

		auto new_player_ptr = players_.AddPlayer(name);

		if (new_player_ptr) {

			auto new_token = generator_.GetNewToken();
			tokens_to_player_id_.insert({new_token, new_player_ptr->GetId()});
			return {new_token, auth::Code::OK};
		}
	}

	return {token::Token(""), Code::MAP_NOT_FOUND};
}


std::pair<player::Player*, Code> Authenticator::GetPlayer(token::Token token) {

	if (!TokenIsCorrect(token)) {
		return {nullptr, Code::TOKEN_IS_INCORRECT};
	}

	if (auto it = tokens_to_player_id_.find(token); it != tokens_to_player_id_.cend()) {
		return {players_.GetPlayer(it->second), Code::OK};
	}

	return {nullptr, Code::PLAYER_NOT_FOUND};
}

player::Players& Authenticator::GetPlayers() {
	return players_;
}


bool Authenticator::TokenIsCorrect(const token::Token& token) {
	return token::TokenIsCorrect(token);
}


} // namespace auth