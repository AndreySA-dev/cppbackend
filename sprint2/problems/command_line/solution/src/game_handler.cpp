#include "game_handler.h"
#include "helper.h"
#include "json_loader.h"
#include "model.h"


#include <iostream>

namespace game_handler {

using namespace std;
using namespace literals;


GameHandler::GameHandler(model::Game& game, net::io_context& ioc, size_t tick_period_ms, bool random_dog_spawn)
	: game_{game}, authenticator_{game}, game_strand_{net::make_strand(ioc)}, tick_timer_{ioc},
	  tick_period_{std::chrono::milliseconds(tick_period_ms)}, is_random_dog_spawn_(random_dog_spawn) {}


std::pair<json::value, game::Code> GameHandler::HandleGetMapsRequest() const {

	return {std::move(GetMapList()), game::Code::OK};
}


std::pair<json::value, game::Code> GameHandler::HandleGetMapRequest(std::string_view map_id) const {

	auto maps = GetMap(std::string(map_id));
	return (maps) ? pair{std::move(*maps), game::Code::OK} : pair{ResponseTemplates::MAP_NOT_FOUND, game::Code::NOT_FOUND};
	
}


pair<json::value, game::Code> GameHandler::HandleGameJoinRequest(std::string_view name, std::string_view map_id_sv) {

	model::Map::Id map_id(std::string{map_id_sv});
	auto [token, code] = authenticator_.AddUser(name, map_id);
	if (code == auth::Code::OK) {

		auto player_ptr = authenticator_.GetUser(token).first;
		auto map_ptr = game_.FindMap(map_id);

		if (!player_ptr || !map_ptr || map_ptr->GetRoads().size() == 0) {
			return {{}, game::Code::ANOTHER_ERROR};
		}

		auto player_id = player_ptr->GetId();
		auto& new_dog = map_ptr->AddDog(model::Dog::Id(*player_id));

		model::Point dog_pos = {0, 0};

		if (is_random_dog_spawn_) {
			// generate Dog random position in random road
			auto& road = map_ptr->GetRoads()[helper::GetRandomNum<int>(0, map_ptr->GetRoads().size())];
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
		} else {
			dog_pos = map_ptr->GetRoads()[0].GetStart();
		}

		new_dog.SetPosition(dog_pos);

		return {{{"authToken"s, *token}, {"playerId"s, *player_id}}, game::Code::OK};

	} else if (code == auth::Code::MAP_NOT_FOUND) {
		return {{}, game::Code::MAP_NOT_FOUND};
	}

	return {{}, game::Code::ANOTHER_ERROR};
}

pair<json::value, game::Code> GameHandler::HandleGetPlayersRequest() const {

	json::object jo;

	size_t i = 0;
	for (const auto& player : authenticator_.GetUsers()) {
		string id_str = std::to_string(i);
		jo.emplace(id_str, json::object{{"name"s, player.GetName()}});
		++i;
	}

	return {jo, game::Code::OK};
}

std::pair<json::value, game::Code> GameHandler::HandleGetStateRequest(user::User* user_ptr) const {

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
			// dog->SetSpeed(0.0); <-- TO-DO
		} else {
			return {{}, game::Code::UNKNOWN_ACTION};
		}
		dog->Start();
		return {json::object{}, game::Code::OK};
	}

	return {json::object{}, game::Code::ANOTHER_ERROR};
}

std::pair<json::value, game::Code> GameHandler::HandleTickRequest(size_t milliseconds) {

	if (milliseconds > 0) {
		UpdateState(std::chrono::milliseconds(milliseconds));
	}
	
	return {{}, game::Code::OK};
}


auth::Authenticator& GameHandler::GetAuthenticator() {
	return authenticator_;
}

net::strand<net::io_context::executor_type>& GameHandler::GetStrand() {
	return game_strand_;
}

void GameHandler::StartTick() {

	if (tick_period_ != 0ns) {
		tick_is_enabled_.store(true);
		ScheduleTick();
	}
}

void GameHandler::StopTick() {
	tick_is_enabled_.store(false);
}

void GameHandler::ScheduleTick() {
	tick_timer_.expires_after(tick_period_);
	auto handle = [self = this->shared_from_this()]() {
		auto now = std::chrono::steady_clock::now();
		auto dur = now - self->prev_upd_time_;
		self->prev_upd_time_ = now;
		self->UpdateState(dur);
		self->ScheduleTick();
	};

	tick_timer_.async_wait([handle, self = this->shared_from_this()](boost::system::error_code ec) {
		if (!ec && self->tick_is_enabled_.load()) {
			net::dispatch(self->game_strand_, handle);
		}
	});
}

void GameHandler::UpdateState(std::chrono::steady_clock::duration duration) {
	// handle all maps
	for (auto& map : game_.GetMaps()) {
		map.Act(duration);
	}
}


json::value GameHandler::GetMapList() const {

	json::array maps_j;

	for (auto& map : game_.GetMaps()) {
		json::object jo;
		json_loader::MapInfoToJSON(map, jo);
		maps_j.push_back(std::move(jo));
	}

	return maps_j;
}


optional<json::value> GameHandler::GetMap(const std::string id) const {

	if (const auto* map_ptr = game_.FindMap(model::Map::Id(std::move(id)))) {
		json::object map_j;
		json_loader::MapToJSON(*map_ptr, map_j);
		return map_j;
	}

	return nullopt;
}

} // namespace game_handler