#pragma once

#include "model.h"
#include "loot_generator.h"

#include <boost/json.hpp>
#include <filesystem>
#include <istream>

namespace json_loader {

namespace json = boost::json;

void DogToJSON(const model::Dog& dog, json::object& jo);

void MapToDogsJSON(const model::Map& map, json::object& jo);

void MapInfoToJSON(const model::Map& map, json::object& jo);

void MapToJSON(const model::Map& Map, json::object& jo);

model::Game LoadGame(std::istream& input);
model::Game LoadGame(const std::filesystem::path& json_path);

} // namespace json_loader
