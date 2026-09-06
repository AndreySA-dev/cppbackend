#pragma once
#include "http_server.h"
#include "json_loader.h"
#include "model.h"

#include <optional>
#include <sstream>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;


struct ContentType {
	ContentType() = delete;
	constexpr static std::string_view TEXT_HTML = "text/html"sv;
	constexpr static std::string_view APP_JSON = "application/json"sv;
};

struct RequestsTexts {
	RequestsTexts() = delete;
	constexpr static std::string_view API = "/api"sv;
	constexpr static std::string_view API_MAPS = "/api/v1/maps"sv;
	constexpr static std::string_view API_ONE_MAP = "/api/v1/maps/"sv;
};

struct ResponseTexts {
	ResponseTexts() = delete;
	constexpr static std::string_view MAP_NOT_FOUND = R"({"code" : "mapNotFound", "message" : "Map not found"})"sv;
	constexpr static std::string_view BAD_REQUEST = R"({"code" : "badRequest", "message" : "Bad request"})"sv;
	constexpr static std::string_view UNKNOWN_REQUST = R"({"code": "unknownRequest", "message": "Unknown request"})"sv;
};


class RequestHandler {
  public:
	explicit RequestHandler(model::Game& game) : game_{game} {}

	RequestHandler(const RequestHandler&) = delete;
	RequestHandler& operator=(const RequestHandler&) = delete;

	template <typename Body, typename Allocator, typename Send>
	void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

		const auto& target = req.target();

		if (target == RequestsTexts::API_MAPS) {
			std::string maps = GetMapList();
			send(GetResponse(maps, http::status::ok, req.keep_alive()));

		} else if (target.starts_with(RequestsTexts::API_ONE_MAP) &&
				   target.size() > RequestsTexts::API_ONE_MAP.size()) {
			// target /api/v1/maps/.... -> get one map by id

			auto map_id = target.substr(RequestsTexts::API_ONE_MAP.size());
			auto maps = GetMap(std::string(map_id));

			if (maps) {
				send(GetResponse(*maps, http::status::ok, req.keep_alive()));
			} else {
				send(GetResponse(ResponseTexts::MAP_NOT_FOUND, http::status::not_found, req.keep_alive()));
			}
		} else if (target.starts_with(RequestsTexts::API))  {
			send(GetResponse(ResponseTexts::BAD_REQUEST, http::status::bad_request, req.keep_alive()));
		} else {
			send(GetResponse(ResponseTexts::UNKNOWN_REQUST, http::status::bad_request, req.keep_alive()));
		}
	}


  private:
	std::string GetMapList();

	std::optional<std::string> GetMap(const std::string& id);

	StringResponse GetResponse(std::string_view text, http::status status, bool keep_alive);

	model::Game& game_;
};

} // namespace http_handler
