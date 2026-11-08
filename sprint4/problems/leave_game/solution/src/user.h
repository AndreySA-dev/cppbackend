#pragma once

#include "model.h"
#include "tagged.h"
#include "token.h"

#include <deque>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>


namespace user {

class User {
  public:
	using Id = util::Tagged<size_t, User>;
	using IdHasher = util::TaggedHasher<User::Id>;
	static inline const Id noid{std::numeric_limits<Id::ValueType>::max()};

	User(std::string name);
	User(std::string name, Id id); //TODO: restrict use not unique Id
	User(const User&) = delete;
	User(User&& other);
	User& operator=(const User& other) = delete;
	User& operator=(User&& other);

	const std::string& GetName() const noexcept;
	Id GetId() const noexcept;
	void SetNextId(size_t next_id); // TODO:  move to Users class

	void SetMap(model::Map* map_ptr);
	model::Map* GetMap();
	const model::Map* GetMap() const;
	model::Dog* GetUserDog();

  private:
	std::string name_;
	Id id_;
	model::Map* map_ptr_ = nullptr;

	static size_t next_id_; // TODO: move to Users class
};

class Users {
	
	using Container = std::list<User>;

  public:
	using iterator = Container::iterator;
	using const_iterator = Container::const_iterator;

	Users() = default;

	User* AddUser(std::string_view name);
	User* AddUser(user::User&& user);
	User* GetUser(User::Id id);
	const User* GetUser(User::Id id) const;
	void RetireUser(user::User::Id id);

	size_t GetSize() const;

	iterator begin();
	const const_iterator begin() const;
	const const_iterator cbegin() const;

	iterator end();
	const const_iterator end() const;
	const const_iterator cend() const;

  private:
	Users(const Users&) = delete;
	Users& operator=(const Users&) = delete;

	// std::deque<User> users_;
	Container users_;
	// std::unordered_map<User::Id, size_t, User::IdHasher> user_id_to_index_;
	// std::unordered_map<std::string_view, User::Id> user_name_to_index_;
	std::unordered_map<User::Id, Container::iterator, User::IdHasher> user_id_to_it_;
	// std::unordered_map<std::string_view, Container::iterator> user_name_to_it_;

	token::TokenHandler generator_;
};


} // namespace user