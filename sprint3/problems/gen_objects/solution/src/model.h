#pragma once
#include "tagged.h"

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace model {

using Dimension = double;
using Coord = Dimension;
using Speed = Dimension;


struct Point {
	Coord x, y;
};

struct Size {
	Dimension width, height;
};

struct Rectangle {
	Point position;
	Size size;
	bool IsPointInBound(Point p) const;
};

struct Offset {
	Dimension dx, dy;
};


enum class Direction : unsigned char { NORTH = 0, EAST, SOUTH, WEST };
const char* const DirectionTitles = "URDL";
char DirectionToChar(model::Direction dir);


class Road {
	struct HorizontalTag {
		explicit HorizontalTag() = default;
	};

	struct VerticalTag {
		explicit VerticalTag() = default;
	};

  public:
	constexpr static HorizontalTag HORIZONTAL{};
	constexpr static VerticalTag VERTICAL{};

	Road(HorizontalTag, Point start, Coord end_x) noexcept : start_{start}, end_{end_x, start.y} {}

	Road(VerticalTag, Point start, Coord end_y) noexcept : start_{start}, end_{start.x, end_y} {}

	Dimension GetLength() const noexcept;

	Rectangle GetRoadRect() const;

	bool IsHorizontal() const noexcept;

	bool IsVertical() const noexcept;

	Point GetStart() const noexcept {
		return start_;
	}

	Point GetEnd() const noexcept {
		return end_;
	}

  private:
	const Dimension SIZE = 0.4;
	Point start_;
	Point end_;
};

class Building {
  public:
	explicit Building(Rectangle bounds) noexcept : bounds_{bounds} {}

	const Rectangle& GetBounds() const noexcept {
		return bounds_;
	}

  private:
	Rectangle bounds_;
};

class Office {
  public:
	using Id = util::Tagged<std::string, Office>;

	Office(Id id, Point position, Offset offset) noexcept : id_{std::move(id)}, position_{position}, offset_{offset} {}

	const Id& GetId() const noexcept {
		return id_;
	}

	Point GetPosition() const noexcept {
		return position_;
	}

	Offset GetOffset() const noexcept {
		return offset_;
	}

  private:
	Id id_;
	Point position_;
	Offset offset_;
};


class Dog {
  public:
	using Id = util::Tagged<size_t, Dog>;

	Dog(Id id);
	Id GetId() const;
	Point GetPosition() const;
	Speed GetSpeed() const;
	void SetSpeed(Speed speed);
	Direction GetDirection() const;
	void SetDirection(model::Direction direction);

	void SetPosition(const Point pos);
	void Start();
	void Stop();
	bool IsMove() const;

  private:
	Point position_ = {0, 0};
	Speed speed_ = 0.0;
	Direction direction_ = Direction::NORTH;
	bool on_move_ = false;
	Id id_;
};


class Map {
  public:
	using Id = util::Tagged<std::string, Map>;
	using Roads = std::vector<Road>;
	using Buildings = std::vector<Building>;
	using Offices = std::vector<Office>;
	using Dogs = std::vector<Dog>;

	Map(Id id, std::string name) noexcept : id_(std::move(id)), name_(std::move(name)) {}

	const Id& GetId() const noexcept {
		return id_;
	}

	void AddRoad(const Road& road);
	void AddBuilding(const Building& building);
	void AddOffice(Office office);
	Dog& AddDog(Dog dog);

	const std::string& GetName() const noexcept;
	const Buildings& GetBuildings() const noexcept;
	const Roads& GetRoads() const noexcept;
	const Offices& GetOffices() const noexcept;
	const Dogs& GetDogs() const noexcept;
	Dogs& GetDogs() noexcept;
	Dog* GetDog(Dog::Id id);
	const Dog* GetDog(Dog::Id id) const;
	std::optional<Speed> GetDogDefaultSpeed() const noexcept;

	void SetDogDefaultSpeed(std::optional<Speed> speed) noexcept;

	void Act(std::chrono::steady_clock::duration duration);

  private:
	using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;
	using DogIdToIndex = std::unordered_map<Dog::Id, size_t, util::TaggedHasher<Dog::Id>>;
	using DogToRoadIdx = std::unordered_map<Dog*, const Road*>;

	void MoveDog(Dog& dog, Dimension distance);
	Id id_;
	std::string name_;
	Roads roads_;
	Buildings buildings_;

	OfficeIdToIndex warehouse_id_to_index_;
	Offices offices_;

	DogIdToIndex dog_id_to_idx_;
	DogToRoadIdx dog_to_road_idx_;
	Dogs dogs_;
	std::optional<Speed> dog_default_speed_ = std::nullopt;
};


class Game {
  public:
	using Maps = std::vector<Map>;

	void AddMap(Map map);

	const Maps& GetMaps() const noexcept;
	Maps& GetMaps() noexcept;

	const Map* FindMap(const Map::Id& id) const noexcept;
	Map* FindMap(const Map::Id& id) noexcept;

	void SetDogDefaultSpeed(std::optional<Speed> speed) noexcept;
	std::optional<Speed> GetDogDefaultSpeed() const noexcept;

	void SetLootSpawnPreiod(double seconds);
	double GetLootSpawnPerion() const;
	void SetLootSpawnProbability(double probabiluty);
	double GetLootSpawnProbability() const;

  private:
	using MapIdHasher = util::TaggedHasher<Map::Id>;
	using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

	std::vector<Map> maps_;
	MapIdToIndex map_id_to_index_;
	std::optional<Speed> dog_default_speed_ = std::nullopt;

	double loot_spawn_period_ = 0.0;
	double loot_spawn_probability_ = 0.0;
};

} // namespace model
