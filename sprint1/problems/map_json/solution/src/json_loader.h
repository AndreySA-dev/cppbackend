#pragma once

#include <boost/json.hpp>
#include <filesystem>

#include "model.h"

namespace json_loader {

namespace json = boost::json;

void MapInfoToJSON(const model::Map& map, json::object& jo);

void MapToJSON(const model::Map& Map, json::object& jo);

model::Game LoadGame(const std::filesystem::path& json_path);

} // namespace json_loader
