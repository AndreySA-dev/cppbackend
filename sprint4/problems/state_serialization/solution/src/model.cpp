#include "model.h"
#include "helper.h"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#ifdef MYDEBUG
#include <iostream>
#endif

namespace model {

using namespace std;
using namespace std::literals;


bool Rectangle::IsPointInBound(Point p) const {
	using namespace helper;
	return LessOrEqual(position.x, p.x) && LessOrEqual(p.x, position.x + size.width) && LessOrEqual(position.y, p.y) &&
		   LessOrEqual(p.y, position.y + size.height);
}


const std::string& Map::GetName() const noexcept {
	return name_;
}


void Map::AddRoad(const Road& road) {
	roads_.emplace_back(road);
	dog_to_road_idx_.clear();
}

const Map::Roads& Map::GetRoads() const noexcept {
	return roads_;
}


void Map::AddBuilding(const Building& building) {
	buildings_.emplace_back(building);
}

const Map::Buildings& Map::GetBuildings() const noexcept {
	return buildings_;
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

const Map::Offices& Map::GetOffices() const noexcept {
	return offices_;
}

Dog& Map::AddDog(Dog::Id id) {

	if (dog_id_to_idx_.contains(id)) {
		throw std::invalid_argument("Duplicate dog");
	}

	const size_t idx = dogs_.size();

	Dog& new_dog = dogs_.emplace_back(id, *dog_bag_capa_);

	try {
		dog_id_to_idx_.emplace(new_dog.GetId(), idx);
		dog_to_road_idx_.clear();
	} catch (...) {
		dogs_.pop_back();
		throw;
	}

	if (GetDogDefaultSpeed()) {
		new_dog.SetSpeed(*GetDogDefaultSpeed());
	}

	return new_dog;
}

Dog* Map::GetDog(Dog::Id id) {

	auto it = dog_id_to_idx_.find(id);
	return it != dog_id_to_idx_.cend() ? &dogs_[it->second] : nullptr;
}

const Dog* Map::GetDog(Dog::Id id) const {

	auto it = dog_id_to_idx_.find(id);
	return it != dog_id_to_idx_.cend() ? &dogs_[it->second] : nullptr;
}

const Map::Dogs& Map::GetDogs() const noexcept {
	return dogs_;
}

Map::Dogs& Map::GetDogs() noexcept {
	return dogs_;
}


void Map::AddLootType(LootType loot_type) {
	size_t idx = loot_types_.size();
	loot_type.id = LootType::Id(idx);
	loot_type_name_idx_[loot_type.name] = idx;
	loot_types_.push_back(std::move(loot_type));
}

LootType* Map::GetLootType(LootType::Id id) {
	return *id < loot_types_.size() ? &loot_types_[*id] : nullptr;
}

const LootType* Map::GetLootType(LootType::Id id) const {
	return *id < loot_types_.size() ? &loot_types_[*id] : nullptr;
}

const LootType* Map::GetLootType(std::string name) const {
	auto it = loot_type_name_idx_.find(name);
	return it != loot_type_name_idx_.cend() ? &loot_types_[it->second] : nullptr;
}

const Map::LootTypes& Map::GetLootTypes() const {
	return loot_types_;
}

size_t Map::GetLootTypeCount() const {
	return loot_types_.size();
}


void Map::AddLoot(LootType::Id type_id, Point pos) {
	auto loot_type_ptr = GetLootType(type_id);
	AddLoot(loot_type_ptr, pos);
}

void Map::AddLoot(const LootType* loot_type, Point pos) {
	size_t loot_id = GetNextId();
	// loots_[Loot::Id{loot_id}] = {loot_type, pos};
	loots_.emplace_back(Loot::Id(loot_id), loot_type, pos);
}

void Map::AddLoot(Loot&& loot) {
	loots_.push_back(std::move(loot));
}

const Map::Loots& Map::GetLoots() const {
	return loots_;
}

// Map::Loots& Map::GetLoots() {
// 	return loots_;
// }

size_t Map::GetLootCount() const {
	return loots_.size();
}

void Map::SetDogDefaultSpeed(std::optional<Speed> speed) noexcept {
	dog_default_speed_ = speed;
}

std::optional<size_t> Map::GetBagCapacity() const {
	return dog_bag_capa_;
}

void Map::SetBagCapacity(size_t capacity) {
	dog_bag_capa_ = capacity;
}

std::optional<Speed> Map::GetDogDefaultSpeed() const noexcept {
	return dog_default_speed_;
}

size_t Map::GetDogCount() const {
	return dogs_.size();
}

void Map::Act(std::chrono::steady_clock::duration duration, size_t new_loot_number) {

	ItemGatherer gatherer(offices_, loots_);

	// move all dogs
	for (auto& dog : GetDogs()) {
		model::Dimension distance = dog.GetSpeed() * std::chrono::duration<double>(duration).count();
		auto start_pos = dog.GetPosition();
		auto new_pos = MoveDog(dog, distance);
#ifdef MYDEBUG
		std::cerr << "Dog move " << start_pos.x << "," << start_pos.y << " " << new_pos.x << "," << new_pos.y
				  << std::endl;
#endif
		gatherer.AddGatherer(&dog, {start_pos.x, start_pos.y}, {new_pos.x, new_pos.y} /*, dog.GetWidth()*/);
	}

	auto events = collision_detector::FindGatherEvents(gatherer);

	// loot idx that has been already captured form road
	set<size_t, std::greater<size_t>> captured_item_indexes;
	for (auto event : events) {

		if (gatherer.IsIdxIsOfficeIdx(event.item_id)) {
			// collision with office, return loot
			auto dog = gatherer.GetDog(event.gatherer_id);
			dog->GetScoreAndFreeBag();

		} else if (!captured_item_indexes.contains(event.item_id)) {
			// if collision not with office, so its loot
			auto dog = gatherer.GetDog(event.gatherer_id);
			if (dog->HasBagFreeSpace()) {
				auto loot = gatherer.GetLootByItemIdx(event.item_id);
				dog->AddLoot(loot);
				captured_item_indexes.insert(event.item_id);
			}
		}
	}

	// remove loot from map
	for (auto captured_loot_idx : captured_item_indexes) {
		loots_.erase(loots_.begin() + captured_loot_idx);
	}

	if (new_loot_number == 0 || GetLootTypeCount() == 0) {
		return;
	}

	while (new_loot_number != 0) {
		// random loot type and random position
		LootType::Id type_id{helper::GetRandomNum<size_t>(0, GetLootTypeCount())};
		auto pos = GetRandPosOnRoad();
		// Point pos{10, 0};
#ifdef MYDEBUG
		cerr << "New loot " << pos.x << "x" << pos.y << endl;
#endif
		AddLoot(type_id, pos);
		--new_loot_number;
	}
}

Point Map::GetRandPosOnRoad() const {
	auto& road = GetRoads()[helper::GetRandomNum<int>(0, GetRoads().size())];
	model::Rectangle rect = road.GetRoadRect();
	model::Point rand_pos;
	rand_pos.x = rect.position.x + rect.size.width * helper::GetRandomNum(0.0, 1.0);
	rand_pos.y = rect.position.y + rect.size.height * helper::GetRandomNum(0.0, 1.0);
	return rand_pos;
}

void Map::SetNextId(size_t value) {
	next_id_ = value;
}

size_t Map::GetCurrentNextId() const {
	return next_id_;
}

Point Map::MoveDog(Dog& dog, Dimension distance) {

	Point start_pos = dog.GetPosition();

	if (!dog.IsMove()) {
		return start_pos;
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
			return dog.GetPosition();
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
	return dog.GetPosition();
}

size_t Map::GetNextId() {
	return next_id_++;
}


Dog::Dog(Id id, size_t bag_capacity) : id_(id), bag_capa_(bag_capacity) {}

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

bool Dog::HasBagFreeSpace() const {
	return bag_.size() < bag_capa_;
}

size_t Dog::LootCount() const {
	return bag_.size();
}

void Dog::AddLoot(Loot loot) {
	return bag_.push_back(std::move(loot));
}

const std::vector<Loot> Dog::GetBag() const {
	return bag_;
}

void Dog::GetScoreAndFreeBag() {
	size_t add_score = std::accumulate(
		bag_.cbegin(), bag_.cend(), 0, [](size_t init, const Loot& loot) { return init + loot.loot_type->value; });
	score_ += add_score;
	bag_.clear();
}

Dimension Dog::GetWidth() const {
	return width_;
}

void Dog::SetBagCapacity(size_t capacity) {
	bag_capa_ = capacity;
}

size_t Dog::GetBagCapacity() const {
	return bag_capa_;
}

void Dog::AddScore(size_t score) {
	score_ += score;
}

size_t Dog::GetScore() const {
	return score_;
}

char DirectionToChar(model::Direction dir) {
	return model::DirectionTitles[static_cast<unsigned char>(dir)];
}

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

ItemGatherer::ItemGatherer(const Map::Offices& offices, const Map::Loots& loots) : offices_(offices), loots_(loots) {}

void ItemGatherer::AddGatherer(Dog* dog, geom::Point2D from, geom::Point2D to) {
	size_t idx = gatherers_.size();
	gatherers_.emplace_back(from, to, dog->GetWidth());
	gatherer_id_to_dog_.insert({idx, dog});
}

size_t ItemGatherer::ItemsCount() const {
	return offices_.size() + loots_.size();
}

collision_detector::Item ItemGatherer::GetItem(size_t idx) const {
	if (idx < offices_.size()) {
		auto& office = offices_[idx];
		return {{office.GetPosition().x, office.GetPosition().y}, office.GetWidth()};
	}
	auto& loot = loots_[idx - offices_.size()];
	return {{loot.position.x, loot.position.y}, loot.width};
}

size_t ItemGatherer::GatherersCount() const {
	return gatherers_.size();
}

cld::Gatherer ItemGatherer::GetGatherer(size_t idx) const {
	return gatherers_[idx];
}

bool ItemGatherer::IsIdxIsOfficeIdx(size_t idx) const {
	return idx < offices_.size();
}

Dog* ItemGatherer::GetDog(size_t idx) {
	auto it = gatherer_id_to_dog_.find(idx);
	return it != gatherer_id_to_dog_.end() ? it->second : nullptr;
}

const Loot& ItemGatherer::GetLootByItemIdx(size_t idx) const {
	assert(idx >= offices_.size());
	return loots_[idx - offices_.size()];
}


Game::Game(std::string data_src_name) : data_src_name_(std::move(data_src_name)) {}

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

	auto it = map_id_to_index_.find(id);
	return it != map_id_to_index_.end() ? &maps_[it->second] : nullptr;
}

Map* Game::FindMap(const Map::Id& id) noexcept {

	auto it = map_id_to_index_.find(id);
	return it != map_id_to_index_.cend() ? &maps_[it->second] : nullptr;
}


void Game::SetDogDefaultSpeed(std::optional<Speed> speed) noexcept {
	dog_default_speed_ = speed;
}


std::optional<Speed> Game::GetDogDefaultSpeed() const noexcept {
	return dog_default_speed_;
}

std::optional<size_t> Game::GetBagDefaultCapacity() noexcept {
	return bag_default_capa_;
}

void Game::SetBagDefaultCapacity(std::optional<size_t> capacity) {
	bag_default_capa_ = capacity;
}

void Game::SetLootSpawnPreiod(double seconds) {
	loot_spawn_period_ = seconds;
}


double Game::GetLootSpawnPeriod() const {
	return loot_spawn_period_;
}


void Game::SetLootSpawnProbability(double probability) {
	loot_spawn_probability_ = probability;
}


double Game::GetLootSpawnProbability() const {
	return loot_spawn_probability_;
}

const std::string& Game::GetDataSourceName() const {
	return data_src_name_;
}

} // namespace model
