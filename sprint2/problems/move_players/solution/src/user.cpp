#include "user.h"

#include <limits>
#include <stdexcept>

namespace user {

using namespace std;
using namespace std::literals;

size_t User::next_id_ = 0;
User::Id User::noid = User::Id{std::numeric_limits<size_t>::max()};

User::User(std::string name) : name_{std::move(name)}, id_{User::Id(next_id_++)} {}

User::User(User&& other) : name_(std::move(other.name_)), id_(std::move(other.id_)) {

	other.id_ = user::User::noid;
}

User& User::operator=(User&& other) {
	name_ = std::move(other.name_);
	id_ = std::move(other.id_);
	other.id_ = user::User::noid;
	return *this;
}

const std::string& User::GetName() const noexcept {
	return name_;
}

User::Id User::GetId() const noexcept {
	return id_;
}


void User::SetMap(const model::Map* map_ptr) {
	map_ptr_ = map_ptr;
}

const model::Map* User::GetMap() const {
	return map_ptr_;
}


User* Users::AddUser(std::string_view name) {


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

User* Users::GetUser(User::Id id) {
	if (auto it = player_id_to_index_.find(id); it != player_id_to_index_.cend()) {
		return &players_[it->second];
	}
	return nullptr;
}


Users::iterator Users::begin() {
	return players_.begin();
}


const Users::const_iterator Users::cbegin() const {
	return players_.cbegin();
}


Users::iterator Users::end() {
	return players_.end();
}


const Users::const_iterator Users::cend() const {
	return players_.cend();
}


} // namespace player
