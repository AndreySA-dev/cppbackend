
#pragma once
#include "authenticator.h"
#include "model.h"
#include "user.h"
#include "token.h"

namespace model {

template <typename Archive>
void serialize(Archive& arch, model::Point& point, [[maybe_unused]] const unsigned version) {
	arch & point.x;
	arch & point.y;
}

} // namespace model

// namespace user {

// template <typename Archive>
// void serialize(Archive& arch, user::User& user, [[maybe_unused]] const unsigned version) {
// 	arch & user.GetId();
// 	arch & user.getName();
// }

// } // namespace user

namespace serialization {


class LootRepr {
  public:
	LootRepr() = default;
	explicit LootRepr(const model::Loot& loot /*, const model::Map* map */);
	model::Loot Restore(const model::Map& map) const;

	template <typename Archive>
	void serialize(Archive& arch, [[maybe_unused]] const unsigned version) {
		arch&* id_;
		arch & loot_type_name_;
		arch & position_;
	}

  private:
	model::Loot::Id id_ = model::Loot::noid;
	std::string loot_type_name_;
	model::Point position_;
};

class DogRepr {

  public:
	DogRepr() = default;
	explicit DogRepr(const model::Dog& dog /*, const model::Map* map */);
	model::Dog Restore(const model::Map& map) const;

	template <typename Archive>
	void serialize(Archive& arch, [[maybe_unused]] const unsigned version) {
		arch & position_;
		arch & speed_;
		arch & direction_;
		arch & on_move_;
		arch&* id_;
		arch & bag_capa_;
		arch & bag_;
		arch & score_;
	}

  private:
	model::Point position_;
	model::Speed speed_;
	model::Direction direction_;
	bool on_move_;
	model::Dog::Id id_ = model::Dog::noid;
	size_t bag_capa_;
	std::vector<LootRepr> bag_;
	size_t score_ = 0;
};


class MapRepr {
  public:
	MapRepr() = default;
	MapRepr(const model::Map& map);
	void Restore(model::Map& map);

	template <typename Archive>
	void serialize(Archive& arch, [[maybe_unused]] const unsigned version) {
		arch & loots_;
		arch & dogs_;
		arch & next_id_;
	}

  private:
	std::vector<LootRepr> loots_;
	std::vector<DogRepr> dogs_;
	size_t next_id_;
};

class GameRepr {
  public:
	GameRepr() = default;
	GameRepr(const model::Game& game);
	void Restore(model::Game& game);

	template <typename Archive>
	void serialize(Archive& arch, [[maybe_unused]] const unsigned version) {
		arch & data_src_name_;
		arch & maps_;
	}

  private:
	std::vector<MapRepr> maps_;
	std::string data_src_name_;
};

// class UserRepr {
//   public:
// 	UserRepr() = default;
// 	UserRepr(const user::User& user);
// 	user::User Restore(auth::Authenticator& authenticator);

// 	template <typename Archive>
// 	void serialize(Archive& arch, [[maybe_unused]] const unsigned version) {
// 		arch & name_;
// 		arch & id_;
// 		arch & *map_id_;
// 	}

//   private:
// 	std::string name_;
// 	user::User::Id id_;
// 	model::Map::Id map_id_;
// 	size_t next_id_;
// };

class OnlinePlayerRepr {
  public:
	OnlinePlayerRepr() = default;
	OnlinePlayerRepr(const user::User& user, token::Token token);
	const std::string& GetUserName() const;
	user::User::Id GetUserId() const;
	model::Map::Id GetMapId() const;
	token::Token GetUserToken() const;
	std::pair<user::User, token::Token> Restore() const;

	template <typename Archive>
	void serialize(Archive& arch, [[maybe_unused]] const unsigned version) {
		arch & name_;
		arch & id_;
		arch & *map_id_;
		arch & *token_;
	}

  private:
	std::string name_;
	user::User::Id id_;
	model::Map::Id map_id_;
	token::Token token_;
};

class AuthRepr {
  public:
	AuthRepr() = default;
	AuthRepr(const auth::Authenticator& authenticator);
	void Restore(auth::Authenticator& authenticator);

	template <typename Archive>
	void serialize(Archive& arch, [[maybe_unused]] const unsigned version) {
		arch & players_;
	}

  private:
	std::vector<OnlinePlayerRepr> players_;
};



} // namespace serialization