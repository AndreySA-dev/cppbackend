#include "json_loader.h"

#include "model.h"
#include <boost/json.hpp>
#include <fstream>
#include <string_view>


namespace json_loader {

using namespace std::literals;

namespace {
	
json::value parse_file(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open file: " + filename);
		exit(1);
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return json::parse(content);
}

} // namespace

std::string JStrToStr(const json::string str) {
	return std::string(str.data(), str.size());
}

model::Point JSONToPoint(const json::value& jv) {
	return {jv.at("x").to_number<int>(), jv.at("y").to_number<int>()};
}

model::Size JSONToSize(const json::value& jv) {
	return {jv.at("h").to_number<int>(), jv.at("w").to_number<int>()};
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


model::Game LoadGame(const std::filesystem::path& json_path) {
	// Загрузить содержимое файла json_path, например, в виде строки
	// Распарсить строку как JSON, используя boost::json::parse
	// Загрузить модель игры из файла
	model::Game game;

	auto json_data = parse_file(json_path);

	for (const auto& json_map : json_data.as_object().at("maps").as_array()) {
		game.AddMap(JSONToMap(json_map.as_object()));
	}

	return game;
}


} // namespace json_loader
