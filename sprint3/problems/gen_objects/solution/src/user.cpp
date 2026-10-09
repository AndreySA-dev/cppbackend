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


void User::SetMap(model::Map* map_ptr) {
	map_ptr_ = map_ptr;
}

model::Map* User::GetMap() {
	return map_ptr_;
}

const model::Map* User::GetMap() const {
	return map_ptr_;
}

model::Dog* User::GetUserDog() {
	
	return map_ptr_ ? map_ptr_->GetDog(model::Dog::Id(*id_)) : nullptr;

}


User* Users::AddUser(std::string_view name) {

	const size_t idx = users_.size();
	decltype(user_id_to_index_.emplace()) add_id_idx_result;
	decltype(user_name_to_index_.emplace()) add_name_idx_result;

	try {

		auto& new_player = users_.emplace_back(std::string(name));
		add_id_idx_result = user_id_to_index_.emplace(new_player.GetId(), idx);
		add_name_idx_result = user_name_to_index_.emplace(new_player.GetName(), idx);

		return &new_player;

	} catch (...) {

		user_id_to_index_.erase(add_id_idx_result.first);
		user_name_to_index_.erase(add_name_idx_result.first);
		throw;
	}

	return nullptr;
}

User* Users::GetUser(User::Id id) {

	auto it = user_id_to_index_.find(id);
	return it != user_id_to_index_.cend() ?  &users_[it->second] : nullptr;
	
}


Users::iterator Users::begin() {
	return users_.begin();
}

const Users::const_iterator Users::begin() const {
	return users_.begin();
}


const Users::const_iterator Users::cbegin() const {
	return users_.cbegin();
}


Users::iterator Users::end() {
	return users_.end();
}

const Users::const_iterator Users::end() const {
	return const_iterator();
}


const Users::const_iterator Users::cend() const {
	return users_.cend();
}


} // namespace player
