#include "player.h"

#include <limits>
#include <stdexcept>

namespace player {

using namespace std;
using namespace std::literals;

size_t Player::next_id_ = 0;
Player::Id Player::noid = Player::Id{std::numeric_limits<size_t>::max()};

Player::Player(std::string name) : name_{std::move(name)}, id_{Player::Id(next_id_++)} {}

Player::Player(Player&& other) : name_(std::move(other.name_)), id_(std::move(other.id_)) {

	other.id_ = player::Player::noid;
}

Player& Player::operator=(Player&& other) {
	name_ = std::move(other.name_);
	id_ = std::move(other.id_);
	other.id_ = player::Player::noid;
	return *this;
}

const std::string& Player::GetName() const noexcept {
	return name_;
}

Player::Id Player::GetId() const noexcept {
	return id_;
}

Player* Players::AddPlayer(std::string_view name) {


	if (player_name_to_index_.find(name) != player_name_to_index_.cend()) {
		throw std::invalid_argument("User with name "s + string(name) + " already exists"s);
	}

	const size_t idx = players_.size();
	decltype(player_id_to_index_.emplace()) add_id_idx_result;
	decltype(player_name_to_index_.emplace()) add_name_idx_result;

	try {

		auto& new_player = players_.emplace_back(std::string(name));
		add_id_idx_result = player_id_to_index_.emplace(new_player.GetId(), idx);
		add_name_idx_result = player_name_to_index_.emplace(new_player.GetName(), idx);

		return &new_player;

	} catch (...) {

		player_id_to_index_.erase(add_id_idx_result.first);
		player_name_to_index_.erase(add_name_idx_result.first);
		throw;
	}

	return nullptr;
}

Player* Players::GetPlayer(Player::Id id) {
	if (auto it = player_id_to_index_.find(id); it != player_id_to_index_.cend()) {
		return &players_[it->second];
	}
	return nullptr;
}


Players::iterator Players::begin() {
	return players_.begin();
}


const Players::const_iterator Players::cbegin() const {
	return players_.cbegin();
}


Players::iterator Players::end() {
	return players_.end();
}


const Players::const_iterator Players::cend() const {
	return players_.cend();
}


} // namespace player
