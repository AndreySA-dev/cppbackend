#pragma once

#include <boost/json.hpp>
#include <filesystem>

#include "model.h"

namespace json_loader {

namespace json = boost::json;

model::Point JSONToPoint(const json::value& jv);

model::Size JSONToSize(const json::value& jv);

model::Building JSONToBuilding(const json::value& jv);

model::Offset JSONToOffset(const json::value& jv);

model::Office JSONToOffice(const json::value& jv);

model::Road JSONToRoad(const json::object& jo);

model::Map JSONToMap(const json::object& jo);


model::Game LoadGame(const std::filesystem::path &json_path);

} // namespace json_loader
