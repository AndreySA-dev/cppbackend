#pragma once
#include "collision_detector.h"
#include "tagged.h"

#include <chrono>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace model {

using Dimension = double;
using Coord = Dimension;
using Speed = Dimension;

// predeclare
class Loot;


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

// struct Color {
// 	char red = 0;
// 	char green = 0;
// 	char blue = 0;
// };

using Color = std::string;

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

	Dimension GetWidth() const {
		return width_;
	}

  private:
	Id id_;
	Point position_;
	Offset offset_;
	static inline const Dimension width_{0.5};
};


class Dog {
  public:
	using Id = util::Tagged<size_t, Dog>;
	static inline const Id noid{std::numeric_limits<Id::ValueType>::max()};

	Dog(Id id, size_t bag_capacity);
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
	bool HasBagFreeSpace() const;
	size_t LootCount() const;
	void AddLoot(Loot loot);
	const std::vector<Loot> GetBag() const;
	void GetScoreAndFreeBag();
	Dimension GetWidth() const;

	void SetBagCapacity(size_t capacity);
	size_t GetBagCapacity() const;

	void AddScore(size_t score);
	size_t GetScore() const;

  private:
	Point position_ = {0, 0};
	Speed speed_ = 0.0;
	Direction direction_ = Direction::NORTH;
	bool on_move_ = false;
	Id id_;

	size_t bag_capa_ = 0;
	std::vector<Loot> bag_;

	size_t score_ = 0;

	static inline const Dimension width_{0.6};
};


struct LootType {
	using Id = util::Tagged<size_t, LootType>;
	static inline const Id noid{std::numeric_limits<Id::ValueType>::max()};

	Id id = noid;
	std::string name;
	std::string file;
	std::string type;
	std::optional<int> rotation = std::nullopt;
	std::optional<Color> color = std::nullopt;
	double scale = 0.0;
	size_t value = 0;
};


struct Loot {
	using Id = util::Tagged<size_t, Loot>;
	static inline const Id noid{std::numeric_limits<Id::ValueType>::max()};

	Id id = noid;
	const LootType* loot_type;
	Point position;
	static inline const double width{0.0};
};


class Map {
  public:
	using Id = util::Tagged<std::string, Map>;
	using Roads = std::vector<Road>;
	using Buildings = std::vector<Building>;
	using Offices = std::vector<Office>;
	using Dogs = std::vector<Dog>;
	using LootTypes = std::vector<LootType>;
	using Loots = std::vector<Loot>;

	Map(Id id, std::string name) noexcept : id_(std::move(id)), name_(std::move(name)) {}

	const Id& GetId() const noexcept {
		return id_;
	}

	const std::string& GetName() const noexcept;
	void AddRoad(const Road& road);
	const Roads& GetRoads() const noexcept;
	void AddBuilding(const Building& building);
	const Buildings& GetBuildings() const noexcept;
	void AddOffice(Office office);
	const Offices& GetOffices() const noexcept;

	Dog& AddDog(Dog::Id id);
	const Dogs& GetDogs() const noexcept;
	Dog* GetDog(Dog::Id id);
	const Dog* GetDog(Dog::Id id) const;
	size_t GetDogCount() const;
	std::optional<Speed> GetDogDefaultSpeed() const noexcept;
	void SetDogDefaultSpeed(std::optional<Speed> speed) noexcept;

	std::optional<size_t> GetBagCapacity() const;
	void SetBagCapacity(size_t capacity);

	void AddLootType(LootType loot_type);
	LootType* GetLootType(LootType::Id id);
	const LootType* GetLootType(LootType::Id id) const;
	const LootType* GetLootType(std::string name) const;
	const LootTypes& GetLootTypes() const;
	size_t GetLootTypeCount() const;

	void AddLoot(LootType::Id type_id, Point pos);
	void AddLoot(const LootType* loot_type, Point pos);
	void AddLoot(Loot&& loot);
	const Loots& GetLoots() const;
	size_t GetLootCount() const;

	void Act(std::chrono::steady_clock::duration duration, size_t new_loot_number);

	Point GetRandPosOnRoad() const;

	void SetNextId(size_t value);
	size_t GetCurrentNextId() const;

  private:
	using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;
	using DogIdToIndex = std::unordered_map<Dog::Id, size_t, util::TaggedHasher<Dog::Id>>;
	using DogToRoadIdx = std::unordered_map<Dog*, const Road*>;
	using LootTypeNameToIdx = std::unordered_map<std::string, size_t>;

	Dogs& GetDogs() noexcept;
	
	// move dog and return new position
	Point MoveDog(Dog& dog, Dimension distance);
	size_t GetNextId();

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

	LootTypes loot_types_;
	LootTypeNameToIdx loot_type_name_idx_;
	Loots loots_;

	std::optional<size_t> dog_bag_capa_ = std::nullopt;

	size_t next_id_ = 0; // get next id unique in map instance
};

namespace cld = collision_detector;

class ItemGatherer : public collision_detector::ItemGathererProvider {
  public:
	ItemGatherer(const Map::Offices& offices, const Map::Loots& loots);
	void AddGatherer(Dog* dog, geom::Point2D from, geom::Point2D to /*, double width*/);

	size_t ItemsCount() const override;
	cld::Item GetItem(size_t idx) const override;
	size_t GatherersCount() const override;
	cld::Gatherer GetGatherer(size_t idx) const override;
	bool IsIdxIsOfficeIdx(size_t idx) const;
	Dog* GetDog(size_t idx);
	const Loot& GetLootByItemIdx(size_t idx) const;

  private:
	const Map::Offices& offices_;
	const Map::Loots& loots_;
	std::vector<cld::Gatherer> gatherers_;
	std::unordered_map<size_t, Dog*> gatherer_id_to_dog_;
};

class Game {
  public:
	using Maps = std::vector<Map>;

	Game(std::string data_source_name);

	void AddMap(Map map);
	const Maps& GetMaps() const noexcept;
	Maps& GetMaps() noexcept;
	const Map* FindMap(const Map::Id& id) const noexcept;
	Map* FindMap(const Map::Id& id) noexcept;

	void SetDogDefaultSpeed(std::optional<Speed> speed) noexcept;
	std::optional<Speed> GetDogDefaultSpeed() const noexcept;

	std::optional<size_t> GetBagDefaultCapacity() noexcept;
	void SetBagDefaultCapacity(std::optional<size_t> capacity);

	void SetLootSpawnPreiod(double seconds);
	double GetLootSpawnPeriod() const;
	void SetLootSpawnProbability(double probabiluty);
	double GetLootSpawnProbability() const;

	const std::string& GetDataSourceName() const;

  private:
	using MapIdHasher = util::TaggedHasher<Map::Id>;
	using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

	std::vector<Map> maps_;
	MapIdToIndex map_id_to_index_;
	std::optional<Speed> dog_default_speed_ = std::nullopt;

	double loot_spawn_period_ = 0.0;
	double loot_spawn_probability_ = 0.0;

	std::optional<size_t> bag_default_capa_ = 3;
	std::string data_src_name_;
};


} // namespace model
