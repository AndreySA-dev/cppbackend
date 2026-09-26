#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;


const std::string& Map::GetName() const noexcept {
	return name_;
}


const Map::Buildings& Map::GetBuildings() const noexcept {
	return buildings_;
}

const Map::Roads& Map::GetRoads() const noexcept {
	return roads_;
}

const Map::Offices& Map::GetOffices() const noexcept {
	return offices_;
}

const Map::Dogs& Map::GetDogs() const noexcept {
	return dogs_;
}


void Map::AddRoad(const Road& road) {
	roads_.emplace_back(road);
}


void Map::AddBuilding(const Building& building) {
	buildings_.emplace_back(building);
}


void Map::AddOffice(Office office) {
	if (warehouse_id_to_index_.contains(office.GetId())) {
		throw std::invalid_argument("Duplicate warehouse");
	}

	const size_t index = offices_.size();
	Office& o = offices_.emplace_back(std::move(office));
	try {
		warehouse_id_to_index_.emplace(o.GetId(), index);
	} catch (...) {
		// Удаляем офис из вектора, если не удалось вставить в unordered_map
		offices_.pop_back();
		throw;
	}
}


Dog& Map::AddDog(Dog dog) {

	if (dog_id_to_idx.contains(dog.GetId())) {
		throw std::invalid_argument("Duplicate dog");
	}

	const size_t idx = dogs_.size();

	Dog& new_dog = dogs_.emplace_back(std::move(dog));

	try {
		dog_id_to_idx.emplace(new_dog.GetId(), idx);
	} catch (...) {
		// Удаляем офис из вектора, если не удалось вставить в unordered_map
		dogs_.pop_back();
		throw;
	}
	return new_dog;
}

Dog* Map::GetDog(Dog::Id id) {

	if (auto it = dog_id_to_idx.find(id); it != dog_id_to_idx.cend()) {
		return &dogs_[it->second];
	}
	return nullptr;
}

const Dog* Map::GetDog(Dog::Id id) const {

	if (auto it = dog_id_to_idx.find(id); it != dog_id_to_idx.cend()) {
		return &dogs_[it->second];
	}
	return nullptr;
}

void Map::SetDogDefaultSpeed(std::optional<Speed> speed) noexcept {
	dog_default_speed_ = speed;
}

std::optional<Speed> Map::GetDogDefaultSpeed() const noexcept {
	return dog_default_speed_;
}


void Game::AddMap(Map map) {

	const size_t index = maps_.size();
	if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
		throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
	} else {
		try {
			maps_.emplace_back(std::move(map));
		} catch (...) {
			map_id_to_index_.erase(it);
			throw;
		}
	}
}

const Map* Game::FindMap(const Map::Id& id) const noexcept {

	if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
		return &maps_.at(it->second);
	}
	return nullptr;
}

Map* Game::FindMap(const Map::Id& id) noexcept {

	if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
		return &maps_.at(it->second);
	}
	return nullptr;
}


void Game::SetDogDefaultSpeed(std::optional<Speed> speed) noexcept {
	dog_default_speed_ = speed;
}


std::optional<Speed> Game::GetDogDefaultSpeed() const noexcept {
	return dog_default_speed_;
}


Dog::Dog(Id id) : id_(id) {}


Dog::Id Dog::GetId() const {
	return id_;
}


Point Dog::GetPosition() const {
	return position_;
}


Speed Dog::GetSpeed() const {
	return speed_;
}

void Dog::SetSpeed(Speed speed) {
	speed_ = speed;
}


Direction Dog::GetDirection() const {
	return direction_;
}

void Dog::SetDirection(model::Direction direction) {
	direction_ = direction;
}


void Dog::SetPosition(const Point pos) {
	position_ = pos;
}


char DirectionToChar(model::Direction dir) {
	return model::DirectionTitles[static_cast<char>(dir)];
}

// Direction ChatToDirection(char dir_char) {
// 	Direction dir = Direction::NORTH;
// 	if (dir_char)
// }

Dimension Road::GetLength() const noexcept {
	return IsHorizontal() ? std::abs(end_.x - start_.x) : std::abs(end_.y - start_.y);
}

} // namespace model
