#include "game_handler.h"
#include "json_loader.h"
#include "model.h"
#include <iostream>

namespace game_handler {

using namespace std;
using namespace literals;


GameHandler::GameHandler(model::Game& game) : game_{game}, authenticator_{game} {}


pair<json::value, game::Code> GameHandler::HandleAPIMapRequest(std::string_view target) {

	if (target == RequestsTexts::API_MAPS) {
		// request -> "/api/v1/maps"

		return {std::move(GetMapList()), game::Code::OK};

	} else if (target.starts_with(RequestsTexts::API_ONE_MAP) && target.size() > RequestsTexts::API_ONE_MAP.size()) {
		// request -> "/api/v1/map/general_map1"

		auto map_id = target.substr(RequestsTexts::API_ONE_MAP.size()); // trim forward part "/api/v1/map/"
		auto maps = GetMap(std::string(map_id));

		if (maps) {
			return {std::move(*maps), game::Code::OK};
		} else {
			return {ResponseTemplates::MAP_NOT_FOUND, game::Code::NOT_FOUND};
		}
		// return maps ? *maps : ResponseTemplates::MAP_NOT_FOUND;
	}

	return {ResponseTemplates::BAD_REQUEST, game::Code::BAD_REQUEST};
}


pair<json::value, game::Code> GameHandler::HandleGameJoinRequest(
	std::string_view name, std::string_view map_id) {

	auto [token, code] = authenticator_.AddPlayer(name, model::Map::Id(string(map_id)));
	json::value jv;

	if (code == auth::Code::OK) {

		auto id = authenticator_.GetPlayer(token).first->GetId();
		return {{{"authToken", *token}, {"playerId", *id}}, game::Code::OK};

	} else if (code == auth::Code::MAP_NOT_FOUND) {

		return {{}, game::Code::MAP_NOT_FOUND};
	}

	return {{}, game::Code::ANOTHER_ERROR};
}

pair<json::value, game::Code> GameHandler::HandleGetPlayersRequest() {
	
	json::object jo;

	size_t i = 0;
	for (auto& player : authenticator_.GetPlayers()) {
		string id_str = std::to_string(i);
		jo.emplace(id_str, json::object{{"name", player.GetName()}});
		++i;
	}

	return {jo, game::Code::OK};

}

auth::Authenticator& GameHandler::GetAuthenticator() {
	return authenticator_;
}


json::value GameHandler::GetMapList() {

	json::array maps_j;

	for (auto& map : game_.GetMaps()) {
		json::object jo;
		json_loader::MapInfoToJSON(map, jo);
		maps_j.push_back(std::move(jo));
	}

	return maps_j;
}


optional<json::value> GameHandler::GetMap(const std::string& id) {

	if (const auto* map_ptr = game_.FindMap(model::Map::Id(id))) {
		json::object map_j;
		json_loader::MapToJSON(*map_ptr, map_j);
		return map_j;
	}

	return nullopt;
}


// json::value GameHandler::AddPlayer(std::string_view name, std::string_view map_id) {

// 	auto [token, code] = authenticator_.AddPlayer(name, model::Map::Id{string{map_id}});
// 	json::value jv;

// 	if (code == game::Code::OK) {

// 		auto id = authenticator_.GetPlayer(token)->GetId();
// 		jv.as_object() = {{"authToken", *token}, {"playerId", *id}};

// 	} else if (code == game::Code::MAP_NOT_FOUND) {
// 		jv.as_object() = {{"code", "mapNotFound"}, {"message", "Map not found"}};
// 	} else {
// 		jv = ResponseTemplates::ANOTHER_ERROR;
// 	}

// 	return jv;
// };


} // namespace game_handler