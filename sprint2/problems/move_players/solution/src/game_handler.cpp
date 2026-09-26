#include "game_handler.h"
#include "helper.h"
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


pair<json::value, game::Code> GameHandler::HandleGameJoinRequest(std::string_view name, std::string_view map_id_sv) {

	model::Map::Id map_id(std::string{map_id_sv});
	auto [token, code] = authenticator_.AddUser(name, map_id);

	if (code == auth::Code::OK) {

		auto player_ptr = authenticator_.GetUser(token).first;
		auto map_ptr = game_.FindMap(map_id);

		if (!player_ptr || !map_ptr) {
			return {{}, game::Code::ANOTHER_ERROR};
		}

		auto player_id = player_ptr->GetId();

		auto& new_dog = map_ptr->AddDog(model::Dog::Id(*player_id));

		// generate Dog random position in random road
		model::Point dog_pos = {0, 0};
		size_t road_num = map_ptr->GetRoads().size();
		if (road_num > 0) {
			auto& road = map_ptr->GetRoads()[helper::GetRandomNum<int>(0, road_num - 1)];
			auto road_len = road.GetLength();
			if (road_len > 0) {
				model::Dimension shift = helper::GetRandomNum<model::Dimension>(0.0, road_len);
				if (road.IsHorizontal()) {
					dog_pos.x = std::min(road.GetStart().x, road.GetEnd().x) + shift;
					dog_pos.y = road.GetStart().y;
				} else {
					dog_pos.y = std::min(road.GetStart().y, road.GetEnd().y) + shift;
					dog_pos.x = road.GetStart().x;
				}
			}
		}
		new_dog.SetPosition(dog_pos);

		return {{{"authToken", *token}, {"playerId", *player_id}}, game::Code::OK};

	} else if (code == auth::Code::MAP_NOT_FOUND) {
		return {{}, game::Code::MAP_NOT_FOUND};
	}

	return {{}, game::Code::ANOTHER_ERROR};
}

pair<json::value, game::Code> GameHandler::HandleGetPlayersRequest() {

	json::object jo;

	size_t i = 0;
	for (auto& player : authenticator_.GetUsers()) {
		string id_str = std::to_string(i);
		jo.emplace(id_str, json::object{{"name", player.GetName()}});
		++i;
	}

	return {jo, game::Code::OK};
}

std::pair<json::value, game::Code> GameHandler::HandleGetStateRequest(user::User* user_ptr) {

	json::object jo;

	auto map_ptr = user_ptr->GetMap();

	json_loader::MapToDogsJSON(*map_ptr, jo);

	return {jo, game::Code::OK};
}

std::pair<json::value, game::Code> GameHandler::HandleActionRequest(
	user::User* user, Actions action, std::string_view prop) {

	if (action == Actions::MOVE) {

		auto dog = user->GetUserDog();
		if (!dog) {
			return {json::object{}, game::Code::ANOTHER_ERROR};
		}
		// U R D L -> set direction
		if (prop == "U"sv) {
			dog->SetDirection(model::Direction::NORTH);
		} else if (prop == "R") {
			dog->SetDirection(model::Direction::EAST);
		} else if (prop == "D") {
			dog->SetDirection(model::Direction::SOUTH);
		} else if (prop == "L") {
			dog->SetDirection(model::Direction::WEST);
		} else if (prop == "") {
			// "" -> STOP DOG
			dog->SetSpeed(0.0);
		} else {
			return {{}, game::Code::UNKNOWN_ACTION};
		}
		return {json::object{}, game::Code::OK};

	}

	return {json::object{}, game::Code::ANOTHER_ERROR};
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