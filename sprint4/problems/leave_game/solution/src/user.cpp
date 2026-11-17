#include "user.h"

#include <limits>
#include <stdexcept>

namespace user {

using namespace std;
using namespace std::literals;

size_t User::next_id_ = 0;

User::User(std::string name) : name_{std::move(name)}, id_{User::Id(next_id_++)} {}

User::User(std::string name, Id id) : name_(std::move(name)), id_(id) {
	next_id_ = *id + 1;
}

User::User(User&& other) : name_(std::move(other.name_)), id_(std::move(other.id_)), map_ptr_(other.map_ptr_) {
	other.id_ = user::User::noid;
}

User& User::operator=(User&& other) {
	name_ = std::move(other.name_);
	id_ = std::move(other.id_);
	other.id_ = user::User::noid;
	map_ptr_ = other.map_ptr_;
	return *this;
}

const std::string& User::GetName() const noexcept {
	return name_;
}

User::Id User::GetId() const noexcept {
	return id_;
}

void User::SetNextId(size_t next_id) {
	next_id_ = next_id;
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
	return AddUser(user::User(std::string(name)));
}

User* Users::AddUser(user::User&& user) {
	decltype(user_id_to_it_.emplace()) add_id_it_result;
	try {
		auto user_id = user.GetId();
		auto new_player_it = users_.insert(users_.end(), std::move(user));
		add_id_it_result = user_id_to_it_.emplace(new_player_it->GetId(), new_player_it);
		if (!add_id_it_result.second) {
			throw std::logic_error("Add user error: User with id " + std::to_string(*user_id) + " already exit.");
		}

		return &*new_player_it;
	} catch (...) {
		user_id_to_it_.erase(add_id_it_result.first);
		throw;
	}
	return nullptr;
}

User* Users::GetUser(User::Id id) {
	auto it = user_id_to_it_.find(id);
	return it != user_id_to_it_.cend() ? &*it->second : nullptr;
}

const User* Users::GetUser(User::Id id) const {
	auto it = user_id_to_it_.find(id);
	return it != user_id_to_it_.cend() ? &*it->second : nullptr;
}

void Users::RetireUser(user::User::Id id) {
	auto user_ptr = GetUser(id);
	if (user_ptr) {
		auto id_idx_it = user_id_to_it_.find(id);
		users_.erase(id_idx_it->second);
		user_id_to_it_.erase(id_idx_it);
	}
}

size_t Users::GetSize() const {
	return users_.size();
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
	return users_.cend();
}


const Users::const_iterator Users::cend() const {
	return users_.cend();
}


} // namespace user
