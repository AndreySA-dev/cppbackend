#include "game_serialization.h"
#include <algorithm>
#include <stdexcept>

namespace serialization {

LootRepr::LootRepr(const model::Loot& loot) : loot_id_(loot.id), position_(loot.position) {
	if (!loot.loot_type) {
		throw std::out_of_range("Loot rerialization error. No loot type.");
	}
	loot_type_id_ = loot.loot_type->id;
}

model::Loot LootRepr::Restore(const model::Map& map) const {
	// auto loot_type = map.GetLootType(loot_type_name_);
	auto loot_type = map.GetLootType(loot_type_id_);
	if (!loot_type) {
		throw std::out_of_range("Loot error. No loot_type id '" + std::to_string(*loot_type_id_) + "'");
	}
	model::Loot result;
	result.id = loot_id_;
	result.loot_type = loot_type;
	result.position = position_;
	return result;
}


DogRepr::DogRepr(const model::Dog& dog /*, const model::Map* map */)
	: position_(dog.GetPosition()), speed_(dog.GetSpeed()), direction_(dog.GetDirection()), on_move_(dog.IsMove()),
	  dog_id_(dog.GetId()), bag_capa_(dog.GetBagCapacity()), score_(dog.GetScore()) {
	bag_.reserve(dog.GetBag().size());
	// for (const auto& loot : dog.GetBag()) { // TO-DO: use std::transform
	// 	bag_.push_back(LootRepr(loot));
	// }
	std::transform(dog.GetBag().begin(), dog.GetBag().end(), std::back_inserter(bag_),
		[](const auto& loot) { return LootRepr(loot); });
}

const model::Dog& DogRepr::Restore(model::Map& map) {
	auto& new_dog = map.AddDog(dog_id_);
	new_dog.SetPosition(position_);
	new_dog.SetSpeed(speed_);
	new_dog.SetDirection(direction_);
	if (on_move_) {
		new_dog.Start();
	}
	new_dog.SetBagCapacity(bag_capa_);
	for (auto& loot_repr : bag_) {
		auto loot = loot_repr.Restore(map);
		new_dog.AddLoot(loot);
	}
	return new_dog;
}


MapRepr::MapRepr(const model::Map& map) : map_id_(map.GetId()), next_id_(map.GetCurrentNextId()) {

	loots_.reserve(map.GetLoots().size());
	std::transform(map.GetLoots().cbegin(), map.GetLoots().cend(), std::back_inserter(loots_),
		[](auto& loot) { return LootRepr(loot); });

	dogs_.reserve(map.GetDogs().size());
	std::transform(map.GetDogs().cbegin(), map.GetDogs().cend(), std::back_inserter(dogs_),
		[](auto& dog) { return DogRepr(dog); });
}

void MapRepr::Restore(model::Map& map) {
	map.SetNextId(next_id_);
	for (auto loot_repr : loots_) {
		map.AddLoot(loot_repr.Restore(map));
	}
	for (auto dog : dogs_) {
		dog.Restore(map);
	}
}

const model::Map::Id& MapRepr::GetMapId() const {
	return map_id_;
}


GameRepr::GameRepr(const model::Game& game) : data_src_name_(game.GetDataSourceName()) {
	std::transform(game.GetMaps().cbegin(), game.GetMaps().cend(), std::back_inserter(maps_),
		[](auto& map) { return MapRepr(map); });
}

void GameRepr::Restore(model::Game& game) {
	if (game.GetDataSourceName() != data_src_name_) {
		throw std::logic_error("Error game load. Initial datasource not same in saved state.");
	}
	if (game.GetMaps().size() != maps_.size()) {
		throw std::logic_error("Error game load. Different maps number in saved state.");
	}
	for (auto& map_repr : maps_) {
		auto map = game.FindMap(map_repr.GetMapId());
		map_repr.Restore(*map);
	}
}


OnlinePlayerRepr::OnlinePlayerRepr(const user::User& user, token::Token token)
	: name_(user.GetName()), id_(user.GetId()), map_id_(user.GetMap()->GetId()), token_(token) {}

const std::string& OnlinePlayerRepr::GetUserName() const {
	return name_;
}

user::User::Id OnlinePlayerRepr::GetUserId() const {
	return id_;
}

model::Map::Id OnlinePlayerRepr::GetMapId() const {
	return map_id_;
}

token::Token OnlinePlayerRepr::GetUserToken() const {
	return token_;
}


AuthRepr::AuthRepr(const auth::Authenticator& authenticator) {
	auto tokens = authenticator.GetTokens();
	std::transform(tokens.cbegin(), tokens.cend(), std::back_inserter(players_), [&authenticator](auto& token) {
		const user::User* user = authenticator.GetUser(token).first;
		auto online_player_repr = OnlinePlayerRepr(*user, token);
		return online_player_repr;
	});
}

void AuthRepr::Restore(auth::Authenticator& authenticator) {
	for (const auto& player : players_) {
		authenticator.RestorePlayer(
			user::User{player.GetUserName(), player.GetUserId()}, player.GetMapId(), player.GetUserToken());
	}
}

} // namespace serialization