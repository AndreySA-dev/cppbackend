#include "json_loader.h"
#include "log.h"
#include "model.h"

#include <fstream>
#include <string_view>
#include <string>

#include <boost/json.hpp>

namespace json_loader {

using namespace std::literals;

namespace {

json::value ParseFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open file: " + filename);
		exit(1);
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return json::parse(content);
}

std::string JStrToStr(const json::string str) {
	return std::string(str.data(), str.size());
}

model::Point JSONToPoint(const json::value& jv) {
	return {jv.at("x").to_number<int>(), jv.at("y").to_number<int>()};
}

model::Size JSONToSize(const json::value& jv) {
	return {jv.at("w").to_number<int>(), jv.at("h").to_number<int>()};
}

model::Rectangle JSONToRectangle(const json::value& jv) {
	return {{JSONToPoint(jv)}, {JSONToSize(jv)}};
}

model::Building JSONToBuilding(const json::value& jv) {
	return model::Building(JSONToRectangle(jv));
}

model::Offset JSONToOffset(const json::value& jv) {
	return {jv.at("offsetX").to_number<int>(), jv.at("offsetY").to_number<int>()};
}

model::Office JSONToOffice(const json::value& jv) {
	const auto& jstr_id = jv.at("id").as_string();
	return {{model::Office::Id{{jstr_id.data(), jstr_id.size()}}}, {JSONToPoint(jv)}, {JSONToOffset(jv)}};
}

model::Road JSONToRoad(const json::object& jo) {
	model::Point p1{jo.at("x0").to_number<int>(), jo.at("y0").to_number<int>()};

	if (const auto& x1 = jo.if_contains("x1")) {
		return model::Road(model::Road::HORIZONTAL, p1, x1->to_number<int>());
	} else if (const auto& y1 = jo.if_contains("y1")) {
		return model::Road(model::Road::VERTICAL, p1, y1->to_number<int>());
	}
	throw std::logic_error("JSON. Incorrect road second point parameter");
}


model::Map JSONToMap(const json::object& jo) {

	model::Map map(model::Map::Id{JStrToStr(jo.at("id").as_string())}, JStrToStr(jo.at("name").as_string()));

	const json::array& roads = jo.at("roads").as_array();
	for (const auto& road : roads) {
		map.AddRoad(JSONToRoad(road.as_object()));
	}

	const json::array& buildings = jo.at("buildings").as_array();
	for (const auto& build : buildings) {
		map.AddBuilding(JSONToBuilding(build.as_object()));
	}

	const json::array& offices = jo.at("offices").as_array();
	for (const auto& office : offices) {
		map.AddOffice(JSONToOffice(office.as_object()));
	}

	return map;
}

void PointToJSON(model::Point point, json::object& jo) {
	jo["x"] = point.x;
	jo["y"] = point.y;
}


void SizeToJSON(model::Size size, json::object& jo) {
	jo["w"] = size.width;
	jo["h"] = size.height;
}


void RectangleToJSON(model::Rectangle rect, json::object& jo) {
	PointToJSON(rect.position, jo);
	SizeToJSON(rect.size, jo);
}


void BuildingToJSON(model::Building building, json::object& jo) {
	RectangleToJSON(building.GetBounds(), jo);
}


void OffsetToJSON(model::Offset offset, json::object& jo) {
	jo["offsetX"] = offset.dx;
	jo["offsetY"] = offset.dy;
}


void OfficeToJSON(const model::Office& office, json::object& jo) {
	jo["id"] = *office.GetId();
	PointToJSON(office.GetPosition(), jo);
	OffsetToJSON(office.GetOffset(), jo);
}


void RoadToJSON(const model::Road& road, json::object& jo) {
	jo["x0"] = road.GetStart().x;
	jo["y0"] = road.GetStart().y;
	if (road.IsHorizontal()) {
		jo["x1"] = road.GetEnd().x;
	} else {
		jo["y1"] = road.GetEnd().y;
	}
}

} // namespace


void DogToJSON(const model::Dog& dog, json::object& jo) {

	jo["id"] = *dog.GetId();
	json::value().emplace_double() = 12.213;
	jo["pos"] = json::array({ json::value(dog.GetPosition().x), json::value(dog.GetPosition().y)});
	jo["speed"] = json::array({dog.GetSpeed().h, dog.GetSpeed().v});
	jo["dir"] = std::string(1, DirectionToChar(dog.GetDirection()));

}


void MapToDogsJSON(const model::Map& map, json::object& jo) {

	json::object dogs_jo;
	for (const auto& dog : map.GetDogs()) {
		json::object dog_jo;
		DogToJSON(dog, dog_jo);
		dogs_jo.emplace(std::to_string(*dog.GetId()), std::move(dog_jo));
	}
	jo["players"] = std::move(dogs_jo);

}


void MapInfoToJSON(const model::Map& map, json::object& jo) {
	jo["id"] = *map.GetId();
	jo["name"] = map.GetName();
}


void MapToJSON(const model::Map& map, json::object& jo) {

	MapInfoToJSON(map, jo);


	auto& jroads = jo.emplace("roads", json::array{}).first->value().as_array();
	for (const auto& road : map.GetRoads()) {
		json::object new_jroad;
		RoadToJSON(road, new_jroad);
		jroads.push_back(std::move(new_jroad));
	}

	auto& jbuildings = jo.emplace("buildings", json::array{}).first->value().as_array();
	for (const auto& building : map.GetBuildings()) {
		json::object new_jbuilding;
		BuildingToJSON(building, new_jbuilding);
		jbuildings.push_back(std::move(new_jbuilding));
	}

	auto& joffices = jo.emplace("offices", json::array{}).first->value().as_array();
	for (const auto& office : map.GetOffices()) {
		json::object new_joffice;
		OfficeToJSON(office, new_joffice);
		joffices.push_back(std::move(new_joffice));
	}
}


model::Game LoadGame(const std::filesystem::path& json_path) {
	// Загрузить содержимое файла json_path, например, в виде строки
	// Распарсить строку как JSON, используя boost::json::parse
	// Загрузить модель игры из файла
	model::Game game;

	auto json_data = ParseFile(json_path);

	for (const auto& json_map : json_data.as_object().at("maps").as_array()) {
		game.AddMap(JSONToMap(json_map.as_object()));
	}

	return game;
}


} // namespace json_loader
