#pragma once

#include "tagged.h"
#include "token.h"

#include <string>
#include <deque>
#include <string_view>
#include <unordered_map>
#include <utility>


namespace player {

class Player {
  public:
	using Id = util::Tagged<size_t, Player>;
	using IdHasher = util::TaggedHasher<Player::Id>;
	static Id noid;

	Player(std::string name);
	Player(const Player&) = delete;
	Player(Player&& other);
	Player& operator=(const Player& other) = delete;
	Player& operator=(Player&& other);

	const std::string& GetName() const noexcept;
	Id GetId() const noexcept;

  private:
	std::string name_;
	Id id_;

	static size_t next_id_;
};

class Players {

  public:

	using iterator = std::deque<Player>::iterator;

	using const_iterator = std::deque<Player>::const_iterator;

	Players() = default;

	Player* AddPlayer(std::string_view name);
	Player* GetPlayer(Player::Id id);
	// const Player* GetPlayer(std::string name) const;

	iterator begin();
	const const_iterator cbegin() const;
	iterator end();
	const const_iterator cend() const;

  private:
	Players(const Players&) = delete;
	Players& operator=(const Players&) = delete;

	std::deque<Player> players_;
	std::unordered_map<Player::Id, size_t, Player::IdHasher> player_id_to_index_;
	std::unordered_map<std::string_view, Player::Id> player_name_to_index_;

	token::TokenHandler generator_;
};


} // namespace player