#pragma once

#include "authenticator.h"
#include "codes.h"
#include "model.h"
#include "user.h"

#include <optional>
#include <string_view>
#include <utility>
#include <atomic>
#include <chrono>

#include <boost/json.hpp>
#include <boost/asio.hpp>

namespace game_handler {

namespace json = boost::json;
namespace net = boost::asio;

using namespace std::literals;

// struct RequestsTexts {
// 	RequestsTexts() = delete;
// 	constexpr static std::string_view API = "/api/"sv;
// 	constexpr static std::string_view API_MAPS = "/api/v1/maps"sv;
// 	constexpr static std::string_view API_ONE_MAP = "/api/v1/maps/"sv;
// 	constexpr static std::string_view API_GAME = "/api/v1/game"sv;
// 	constexpr static std::string_view API_GAME_JOIN = "/api/v1/game/join"sv;
// };


struct ResponseTemplates {
	ResponseTemplates() = delete;
	inline static json::value MAP_NOT_FOUND{{"code", "mapNotFound"}, {"message", "Map not found"}};
	inline static json::value BAD_REQUEST{{"code", "badRequest"}, {"message", "Bad request"}};
	inline static json::value ANOTHER_ERROR{{"code", "anotherError"}, {"message", "Another error"}};
};

enum class Actions {
	MOVE,
	NOTHING
};

class GameHandler : public std::enable_shared_from_this<GameHandler> {
  public:
	explicit GameHandler(model::Game& game, net::io_context& ioc, size_t tick_period_ms, bool random_dog_spawn);

	std::pair<json::value, game::Code> HandleGetMapsRequest() const;
	std::pair<json::value, game::Code> HandleGetMapRequest(std::string_view map_id) const;
	std::pair<json::value, game::Code> HandleGameJoinRequest(std::string_view name, std::string_view map_id_sv);
	std::pair<json::value, game::Code> HandleGetPlayersRequest() const;
	std::pair<json::value, game::Code> HandleGetStateRequest(user::User* user_ptr) const;
	std::pair<json::value, game::Code> HandleActionRequest(user::User* user_ptr, Actions action, std::string_view prop);
	std::pair<json::value, game::Code> HandleTickRequest(size_t milliseconds);

	auth::Authenticator& GetAuthenticator();
	net::strand<net::io_context::executor_type>& GetStrand();
	void StartTick();
	void StopTick();
	void UpdateState(std::chrono::steady_clock::duration dur);
	// void OnTickTimer(boost::system::error_code ec);
	
	private:
	
	json::value GetMapList() const ;
	void ScheduleTick();

	std::optional<json::value> GetMap(const std::string id) const ;

	// json::value AddPlayer(std::string_view name, std::string_view id);

	model::Game& game_;
	auth::Authenticator authenticator_;
	net::strand<net::io_context::executor_type>  game_strand_;

	net::steady_timer tick_timer_;
	const std::chrono::steady_clock::duration tick_period_{0ms};
	std::atomic<bool> tick_is_enabled_{false};
	std::chrono::steady_clock::time_point prev_upd_time_;
	bool is_random_dog_spawn_ = false;
};


} // namespace game_handler