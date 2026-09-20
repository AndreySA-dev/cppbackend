#pragma once

#include "authenticator.h"
#include "codes.h"
#include "model.h"
#include "player.h"

#include <boost/json.hpp>
#include <optional>
#include <string_view>
#include <utility>

namespace game_handler {

namespace json = boost::json;

using namespace std::literals;

struct RequestsTexts {
	RequestsTexts() = delete;
	constexpr static std::string_view API = "/api/"sv;
	constexpr static std::string_view API_MAPS = "/api/v1/maps"sv;
	constexpr static std::string_view API_ONE_MAP = "/api/v1/maps/"sv;
	constexpr static std::string_view API_GAME = "/api/v1/game"sv;
	constexpr static std::string_view API_GAME_JOIN = "/api/v1/game/join"sv;
};


struct ResponseTemplates {
	ResponseTemplates() = delete;
	inline static json::value MAP_NOT_FOUND{{"code", "mapNotFound"}, {"message", "Map not found"}};
	inline static json::value BAD_REQUEST{{"code", "badRequest"}, {"message", "Bad request"}};
	inline static json::value ANOTHER_ERROR{{"code", "anotherError"}, {"message", "Another error"}};
};

class GameHandler {
  public:
	explicit GameHandler(model::Game& game);
	std::pair<json::value, game::Code> HandleAPIMapRequest(std::string_view target);
	std::pair<json::value, game::Code> HandleGameJoinRequest(std::string_view name, std::string_view map_id);
	std::pair<json::value, game::Code> HandleGetPlayersRequest();

	auth::Authenticator& GetAuthenticator();


  private:
	json::value GetMapList();
	std::optional<json::value> GetMap(const std::string& id);

	// json::value AddPlayer(std::string_view name, std::string_view id);

	model::Game& game_;
	auth::Authenticator authenticator_;
};


} // namespace game_handler