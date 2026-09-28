#pragma once

#include "tagged.h"
#include "token.h"
#include "model.h"

#include <string>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <utility>


namespace user {

class User {
  public:
	using Id = util::Tagged<size_t, User>;
	using IdHasher = util::TaggedHasher<User::Id>;
	static Id noid;

	User(std::string name);
	User(const User&) = delete;
	User(User&& other);
	User& operator=(const User& other) = delete;
	User& operator=(User&& other);

	const std::string& GetName() const noexcept;
	Id GetId() const noexcept;

	void SetMap(model::Map* map_ptr);
	model::Map* GetMap();
	const model::Map* GetMap() const;
	model::Dog* GetUserDog();

  private:
	std::string name_;
	Id id_;
	model::Map* map_ptr_ = nullptr;

	static size_t next_id_;
};

class Users {

  public:

	using iterator = std::deque<User>::iterator;

	using const_iterator = std::deque<User>::const_iterator;

	Users() = default;

	User* AddUser(std::string_view name);
	User* GetUser(User::Id id);
	// const Player* GetPlayer(std::string name) const;

	iterator begin();
	const const_iterator cbegin() const;
	iterator end();
	const const_iterator cend() const;

  private:
	Users(const Users&) = delete;
	Users& operator=(const Users&) = delete;

	std::deque<User> players_;
	std::unordered_map<User::Id, size_t, User::IdHasher> player_id_to_index_;
	std::unordered_map<std::string_view, User::Id> player_name_to_index_;

	token::TokenHandler generator_;
};


} // namespace player