#include "model.h"
#include "helper.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

namespace model {

using namespace std;
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

Map::Dogs& Map::GetDogs() noexcept {
	return dogs_;
}


void Map::AddRoad(const Road& road) {
	roads_.emplace_back(road);
	dog_to_road_idx_.clear();
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

	if (dog_id_to_idx_.contains(dog.GetId())) {
		throw std::invalid_argument("Duplicate dog");
	}

	const size_t idx = dogs_.size();

	Dog& new_dog = dogs_.emplace_back(std::move(dog));

	try {
		dog_id_to_idx_.emplace(new_dog.GetId(), idx);
		dog_to_road_idx_.clear();
	} catch (...) {
		// Удаляем офис из вектора, если не удалось вставить в unordered_map
		dogs_.pop_back();
		throw;
	}

	if (GetDogDefaultSpeed()) {
		new_dog.SetSpeed(*GetDogDefaultSpeed());
	}

	return new_dog;
}

Dog* Map::GetDog(Dog::Id id) {

	if (auto it = dog_id_to_idx_.find(id); it != dog_id_to_idx_.cend()) {
		return &dogs_[it->second];
	}
	return nullptr;
}

const Dog* Map::GetDog(Dog::Id id) const {

	if (auto it = dog_id_to_idx_.find(id); it != dog_id_to_idx_.cend()) {
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

void Map::Act(std::chrono::steady_clock::duration duration) {

	for (auto& dog : GetDogs()) {
		auto s = std::chrono::duration<double>(duration).count();
		// std::cerr << "Move dog " << *dog.GetId() << ". Speed-" << dog.GetSpeed() << ". Seconds - " << s
		// 		  << std::endl; // ============= DBG LOG !!!
		model::Dimension distance = dog.GetSpeed() * s;
		// std::cerr << "Move dog " << *dog.GetId() << " distance - " << distance
		// 		  << std::endl; // ============= DBG LOG !!!
		MoveDog(dog, distance);
	}
}

void Map::MoveDog(Dog& dog, Dimension distance) {

	// using namespace helper;

	if (!dog.IsMove()) {
		return;
	}
	
	Point curr_pos = dog.GetPosition();

	// evaluate new position
	Point expected_pos = dog.GetPosition();
	if (dog.GetDirection() == Direction::NORTH) {
		expected_pos.y -= distance;
	} else if (dog.GetDirection() == Direction::EAST) {
		expected_pos.x += distance;
	} else if (dog.GetDirection() == Direction::SOUTH) {
		expected_pos.y += distance;
	} else if (dog.GetDirection() == Direction::WEST) {
		expected_pos.x -= distance;
	}

	
	const Road* curr_road = nullptr;
	const Road* prev_road = nullptr;
	unordered_set<const Road*> visited_roads;
	
	// try find current cached road for dog position
	if (auto road_it = dog_to_road_idx_.find(&dog); road_it != dog_to_road_idx_.cend()) {
		curr_road = road_it->second;
		
	}
	
	while (visited_roads.size() != GetRoads().size()) {

		// road may be taked from cache for first loop
		if (!curr_road) {
			// find road for current position in remaining roads
			for (auto& road : GetRoads()) {
				if (!visited_roads.contains(&road) && road.GetRoadRect().IsPointInBound(curr_pos)) {
					curr_road = &road;
					break;
				}
			}
		}

		if (!curr_road) {
			// last pos not finded in remainig roads, dog bumped in edge of road
			dog.Stop();
			break;
		}

		// most likely case
		auto rect = curr_road->GetRoadRect();
		if (rect.IsPointInBound(expected_pos)) {

			dog_to_road_idx_[&dog] = curr_road;
			dog.SetPosition(expected_pos);
			return;
		}

		// Dog bumped into the road edge
		// Get edge position
		if (dog.GetDirection() == Direction::NORTH) {
			curr_pos.y = rect.position.y;
		} else if (dog.GetDirection() == Direction::EAST) {
			curr_pos.x = rect.position.x + rect.size.width;
		} else if (dog.GetDirection() == Direction::SOUTH) {
			curr_pos.y = rect.position.y + rect.size.height;
		} else if (dog.GetDirection() == Direction::WEST) {
			curr_pos.x = rect.position.x;
		}

		// mark this road as visited
		visited_roads.insert(curr_road);

		prev_road = curr_road;
		curr_road = nullptr;

	}

	dog.SetPosition(curr_pos);
	dog_to_road_idx_[&dog] = prev_road;
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

const Game::Maps& Game::GetMaps() const noexcept {
	return maps_;
}

Game::Maps& Game::GetMaps() noexcept {
	return maps_;
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

void Dog::Start() {
	on_move_ = true;
}

void Dog::Stop() {
	on_move_ = false;
}

bool Dog::IsMove() const {
	return on_move_;
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

Rectangle Road::GetRoadRect() const {

	const auto [left, right] = std::minmax(start_.x, end_.x);
	const auto [top, bottom] = std::minmax(start_.y, end_.y);
	return {{left - SIZE, top - SIZE}, {right - left + SIZE * 2, bottom - top + SIZE * 2}};
}

bool Road::IsHorizontal() const noexcept {
	return start_.y == end_.y;
}

bool Road::IsVertical() const noexcept {
	return start_.x == end_.x;
}

bool Rectangle::IsPointInBound(Point p) const {
	using namespace helper;
	return LessOrEqual(position.x, p.x) && LessOrEqual(p.x, position.x + size.width) && LessOrEqual(position.y, p.y) &&
		   LessOrEqual(p.y, position.y + size.height);
}

} // namespace model
