#pragma once

#include "model.h"
#include "loot_generator.h"

#include <boost/json.hpp>
#include <filesystem>
#include <istream>

namespace json_loader {

namespace json = boost::json;

json::value ParseFile(std::istream& input);
json::value ParseFile(const std::string& filename);

model::Map JSONToMap(const json::object& jo);
void MapToDogsJSON(const model::Map& map, json::object& jo);
void MapToLootJSON(const model::Map& map, json::object& jo);

void MapInfoToJSON(const model::Map& map, json::object& jo);

void MapToJSON(const model::Map& Map, json::object& jo);

model::Game LoadGame(std::istream& input, std::string data_source_name);
model::Game LoadGame(const std::filesystem::path& json_path, std::string data_source_name);

} // namespace json_loader
