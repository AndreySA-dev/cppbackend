#include "json_loader.h"
#include "loot_generator.h"
#include "model.h"

#include <boost/json.hpp>
#include <fstream>
#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace json_loader {

using namespace std::literals;

namespace keys {
	static const auto x = "x"s;
	static const auto y = "y"s;
	static const auto weight = "w"s;
	static const auto height = "h"s;
	static const auto pos = "pos"s;
	static const auto dir = "dir"s;
	static const auto speed = "speed"s;
	static const auto offsetX = "offsetX"s;
	static const auto offsetY = "offsetY"s;
	static const auto id = "id"s;
	static const auto x0 = "x0"s;
	static const auto y0 = "y0"s;
	static const auto x1 = "x1"s;
	static const auto y1 = "y1"s;
	static const auto rotation = "rotation"s;

	static const auto name = "name"s;
	static const auto file = "file"s;
	static const auto type = "type"s;
	static const auto color = "color"s;
	static const auto scale = "scale"s;
	static const auto value = "value"s;
	static const auto maps = "maps"s;
	static const auto lootGeneratorConfig = "lootGeneratorConfig"s;
	static const auto period = "period"s;
	static const auto probability = "probability"s;
	static const auto players = "players"s;
	static const auto lostObjects = "lostObjects"s;
	static const auto roads = "roads"s;
	static const auto buildings = "buildings"s;
	static const auto offices = "offices"s;
	static const auto lootTypes = "lootTypes"s;

	static const auto dogSpeed = "dogSpeed"s;
	static const auto dogRetirementTime = "dogRetirementTime"s;
	static const auto bagCapacity = "bagCapacity"s;
	static const auto defaultBagCapacity = "defaultBagCapacity"s;
	static const auto defaultDogSpeed = "defaultDogSpeed"s;
	static const auto bag = "bag"s;
	static const auto score = "score"s;
}

json::value ParseFile(std::istream& input) {

	std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
	return json::parse(content);
}

json::value ParseFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open file: " + filename);
		exit(1);
	}

	return ParseFile(file);
}

std::string JStrToStr(const json::string str) {
	return std::string(str.data(), str.size());
}

model::Point JSONToPoint(const json::value& jv) {
	return {jv.at(keys::x).to_number<model::Dimension>(), jv.at(keys::y).to_number<model::Dimension>()};
}

model::Size JSONToSize(const json::value& jv) {
	return {jv.at(keys::weight).to_number<model::Dimension>(), jv.at(keys::height).to_number<model::Dimension>()};
}

model::Rectangle JSONToRectangle(const json::value& jv) {
	return {{JSONToPoint(jv)}, {JSONToSize(jv)}};
}

model::Building JSONToBuilding(const json::value& jv) {
	return model::Building(JSONToRectangle(jv));
}

model::Offset JSONToOffset(const json::value& jv) {
	return {jv.at(keys::offsetX).to_number<model::Dimension>(), jv.at(keys::offsetY).to_number<model::Dimension>()};
}

model::Office JSONToOffice(const json::value& jv) {
	const auto& jstr_id = jv.at(keys::id).as_string();
	return {{model::Office::Id{{jstr_id.data(), jstr_id.size()}}}, {JSONToPoint(jv)}, {JSONToOffset(jv)}};
}

model::Road JSONToRoad(const json::object& jo) {
	model::Point p1{jo.at(keys::x0).to_number<model::Dimension>(), jo.at(keys::y0).to_number<model::Dimension>()};

	if (const auto& x1 = jo.if_contains(keys::x1)) {
		return model::Road(model::Road::HORIZONTAL, p1, x1->to_number<model::Dimension>());
	} else if (const auto& y1 = jo.if_contains(keys::y1)) {
		return model::Road(model::Road::VERTICAL, p1, y1->to_number<model::Dimension>());
	}
	throw std::logic_error("JSON. Incorrect road second point parameter");
}


model::LootType JSONToLootType(const json::object jo) {

	model::LootType type;

	type.name = jo.at(keys::name).as_string();
	type.file = jo.at(keys::file).as_string();
	type.type = jo.at(keys::type).as_string();

	auto rotation = jo.if_contains(keys::rotation);
	if (rotation) {
		type.rotation = rotation->to_number<double>();
	}

	auto color = jo.if_contains(keys::color);
	if (color) {
		type.color = color->as_string();
	}

	auto scale = jo.if_contains(keys::scale);
	if (scale) {
		type.scale = scale->to_number<double>();
	}

	auto value = jo.if_contains(keys::value);
	if (value) {
		type.value = value->to_number<size_t>();
	}

	return type;
}


model::Map JSONToMap(const json::object& jo) {

	model::Map map(model::Map::Id{JStrToStr(jo.at(keys::id).as_string())}, JStrToStr(jo.at(keys::name).as_string()));

	const json::array& roads_ja = jo.at(keys::roads).as_array();
	for (const auto& road_jo : roads_ja) {
		map.AddRoad(JSONToRoad(road_jo.as_object()));
	}

	const json::array& buildings = jo.at(keys::buildings).as_array();
	for (const auto& build : buildings) {
		map.AddBuilding(JSONToBuilding(build.as_object()));
	}

	const json::array& offices = jo.at(keys::offices).as_array();
	for (const auto& office : offices) {
		map.AddOffice(JSONToOffice(office.as_object()));
	}

	const json::value* dog_speed = jo.if_contains(keys::dogSpeed);
	if (dog_speed) {
		model::Dimension speed = dog_speed->to_number<model::Dimension>();
		map.SetDogDefaultSpeed(speed);
	}

	const json::value* dog_bag_capacity = jo.if_contains(keys::bagCapacity);
	if (dog_bag_capacity) {
		map.SetBagCapacity(dog_bag_capacity->to_number<size_t>());
	}

	const json::value* loot_types = jo.if_contains(keys::lootTypes);
	if (loot_types && loot_types->is_array()) {
		for (const auto& type : loot_types->as_array()) {
			map.AddLootType(JSONToLootType(type.as_object()));
		}
	}

	return map;
}


void PointToJSON(model::Point point, json::object& jo) {
	jo[keys::x] = point.x;
	jo[keys::y] = point.y;
}


void SizeToJSON(model::Size size, json::object& jo) {
	jo[keys::weight] = size.width;
	jo[keys::height] = size.height;
}


void RectangleToJSON(model::Rectangle rect, json::object& jo) {
	PointToJSON(rect.position, jo);
	SizeToJSON(rect.size, jo);
}


void BuildingToJSON(model::Building building, json::object& jo) {
	RectangleToJSON(building.GetBounds(), jo);
}


void OffsetToJSON(model::Offset offset, json::object& jo) {
	jo[keys::offsetX] = offset.dx;
	jo[keys::offsetY] = offset.dy;
}


void OfficeToJSON(const model::Office& office, json::object& jo) {
	jo[keys::id] = *office.GetId();
	PointToJSON(office.GetPosition(), jo);
	OffsetToJSON(office.GetOffset(), jo);
}


void RoadToJSON(const model::Road& road, json::object& jo) {
	jo[keys::x0] = road.GetStart().x;
	jo[keys::y0] = road.GetStart().y;
	if (road.IsHorizontal()) {
		jo[keys::x1] = road.GetEnd().x;
	} else {
		jo[keys::y1] = road.GetEnd().y;
	}
}

void DogToJSON(const model::Dog& dog, json::object& jo) {

	jo[keys::pos] = json::array({json::value(dog.GetPosition().x), json::value(dog.GetPosition().y)});

	model::Dimension v_speed = 0;
	model::Dimension h_speed = 0;
	if (dog.IsMove()) {

		if (dog.GetDirection() == model::Direction::NORTH) {
			v_speed = dog.GetSpeed() * -1;
		} else if (dog.GetDirection() == model::Direction::EAST) {
			h_speed = dog.GetSpeed();
		} else if (dog.GetDirection() == model::Direction::SOUTH) {
			v_speed = dog.GetSpeed();
		} else if (dog.GetDirection() == model::Direction::WEST) {
			h_speed = dog.GetSpeed() * -1;
		}
	}
	jo[keys::speed] = json::array({h_speed, v_speed});

	jo[keys::dir] = std::string(1, DirectionToChar(dog.GetDirection()));

	json::array bag_ja;
	bag_ja.reserve(dog.GetBag().size());
	for (auto loot : dog.GetBag()) {
		bag_ja.push_back(json::object{{"id", *loot.id}, {"type", *loot.loot_type->id}});
	}
	jo[keys::bag] = std::move(bag_ja);

	jo[keys::score] = dog.GetScore();
}


void LootTypeToJSON(const model::LootType type, json::object& jo) {
	jo[keys::name] = type.name;
	jo[keys::file] = type.file;
	jo[keys::type] = type.type;
	if (type.rotation) {
		jo[keys::rotation] = *type.rotation;
	}
	if (type.color) {
		jo[keys::color] = *type.color;
	}
	jo[keys::scale] = type.scale;
	jo[keys::value] = type.value;
}

void LootToJSON(const model::Loot loot, json::object& jo) {
	jo[keys::type] = *loot.loot_type->id;
	jo[keys::pos] = json::array({loot.position.x, loot.position.y});
}

model::Game LoadGame(const json::value& jv, std::string data_source_name) {

	model::Game game(std::move(data_source_name));
	auto& cfg_jo = jv.as_object();
	const json::value* def_dog_speed = cfg_jo.if_contains(keys::defaultDogSpeed);
	if (def_dog_speed) {
		model::Dimension speed = def_dog_speed->to_number<model::Dimension>();
		game.SetDogDefaultSpeed(speed);
	}

	const json::value* capa_jv = cfg_jo.if_contains(keys::defaultBagCapacity);
	if (capa_jv) {
		game.SetBagDefaultCapacity(capa_jv->to_number<size_t>());
	}

	for (const auto& json_map : cfg_jo.at(keys::maps).as_array()) {
		auto new_map = JSONToMap(json_map.as_object());

		if (!new_map.GetDogDefaultSpeed() && game.GetDogDefaultSpeed()) {
			new_map.SetDogDefaultSpeed(game.GetDogDefaultSpeed());
		}

		if (!new_map.GetBagCapacity() && game.GetBagDefaultCapacity()) {
			new_map.SetBagCapacity(*game.GetBagDefaultCapacity());
		}

		game.AddMap(new_map);
	}

	// Load loot parameters
	auto loot_jo = cfg_jo.if_contains(keys::lootGeneratorConfig);
	if (loot_jo && loot_jo->is_object()) {
		game.SetLootSpawnPreiod(loot_jo->at(keys::period).as_double());
		game.SetLootSpawnProbability(loot_jo->at(keys::probability).as_double());
	}

	// Load player retirement time parameter
	auto retirement_time_jv = cfg_jo.if_contains(keys::dogRetirementTime);
	if (retirement_time_jv && retirement_time_jv->is_double()) {
		game.SetDogRetirementTime(retirement_time_jv->as_double());
	}

	return game;
}


void MapToDogsJSON(const model::Map& map, json::object& jo) {

	json::object dogs_jo;
	for (const auto& dog : map.GetDogs()) {
		json::object dog_jo;
		DogToJSON(dog, dog_jo);
		dogs_jo.emplace(std::to_string(*dog.GetId()), std::move(dog_jo));
	}
	jo[keys::players] = std::move(dogs_jo);
}

void MapToLootJSON(const model::Map& map, json::object& jo) {

	json::object loots_jo;
	for (const auto loot : map.GetLoots()) {
		json::object loot_jo;
		LootToJSON(loot, loot_jo);
		loots_jo.emplace(std::to_string(*loot.id), std::move(loot_jo));
	}
	jo[keys::lostObjects] = std::move(loots_jo);
}


void MapInfoToJSON(const model::Map& map, json::object& jo) {
	jo[keys::id] = *map.GetId();
	jo[keys::name] = map.GetName();
}


void MapToJSON(const model::Map& map, json::object& jo) {

	MapInfoToJSON(map, jo);

	auto& jroads = jo.emplace(keys::roads, json::array{}).first->value().as_array();
	for (const auto& road : map.GetRoads()) {
		json::object new_jroad;
		RoadToJSON(road, new_jroad);
		jroads.push_back(std::move(new_jroad));
	}

	auto& jbuildings = jo.emplace(keys::buildings, json::array{}).first->value().as_array();
	for (const auto& building : map.GetBuildings()) {
		json::object new_jbuilding;
		BuildingToJSON(building, new_jbuilding);
		jbuildings.push_back(std::move(new_jbuilding));
	}

	auto& joffices = jo.emplace(keys::offices, json::array{}).first->value().as_array();
	for (const auto& office : map.GetOffices()) {
		json::object new_joffice;
		OfficeToJSON(office, new_joffice);
		joffices.push_back(std::move(new_joffice));
	}

	json::array loot_types_ja;
	for (const auto& type : map.GetLootTypes()) {
		json::object new_type_jo;
		LootTypeToJSON(type, new_type_jo);
		loot_types_ja.push_back(std::move(new_type_jo));
	}
	jo.emplace(keys::lootTypes, std::move(loot_types_ja));
}


model::Game LoadGame(std::istream& input, std::string data_source_name) {
	return LoadGame(ParseFile(input), std::move(data_source_name));
}

model::Game LoadGame(const std::filesystem::path& json_path, std::string data_source_name) {
	// Загрузить содержимое файла json_path, например, в виде строки
	// Распарсить строку как JSON, используя boost::json::parse
	// Загрузить модель игры из файла

	const auto cfg_jv = ParseFile(json_path);
	return LoadGame(cfg_jv, std::move(data_source_name));
}


} // namespace json_loader
